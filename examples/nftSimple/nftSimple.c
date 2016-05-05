
// ============================================================================
// Includes
// ============================================================================
#include <AR2/tracking.h>
#ifdef ANDROID
#include <AR/gsub_es.h>
#else
#include <AR/gsub_lite.h>
#endif

#include "ARMarkerNFT.h"
#include "TrackingSub.h"
#include "NFTSimple.h"

// ============================================================================
// Global variables
// ============================================================================

// Markers.
ARMarkerNFT     *g_pMarkersNFT      = NULL;
int             g_nMarkersNFTCount  = 0;

// NFT.
THREAD_HANDLE_T *g_threadHandle     = NULL;
AR2HandleT      *g_ar2Handle        = NULL;
KpmHandle       *g_kpmHandle        = NULL;
int             g_surfaceSetCount   = 0;
AR2SurfaceSetT  *g_surfaceSet[PAGES_MAX];

// Modifies globals: kpmHandle, ar2Handle.
int InitNFT(ARParamLT *cparamLT, AR_PIXEL_FORMAT pixFormat)
{
    ARLOGd("Initialising NFT.\n");

    //
    // NFT init.
    //

    // KPM init.
    g_kpmHandle = kpmCreateHandle(cparamLT, pixFormat);
    if (!g_kpmHandle)
    {
        ARLOGe("Error: kpmCreateHandle.\n");
        return (FALSE);
    }

    // kpmSetProcMode( kpmHandle, KpmProcHalfSize );

    // AR2 init.
    if ((g_ar2Handle = ar2CreateHandle(cparamLT, pixFormat, AR2_TRACKING_DEFAULT_THREAD_NUM)) == NULL)
    {
        ARLOGe("Error: ar2CreateHandle.\n");
        kpmDeleteHandle(&g_kpmHandle);
        return (FALSE);
    }

    if (threadGetCPU() <= 1)
    {
        ARLOGi("Using NFT tracking settings for a single CPU.\n");
        ar2SetTrackingThresh(g_ar2Handle, 5.0);
        ar2SetSimThresh(g_ar2Handle, 0.50);
        ar2SetSearchFeatureNum(g_ar2Handle, 16);
        ar2SetSearchSize(g_ar2Handle, 6);
        ar2SetTemplateSize1(g_ar2Handle, 6);
        ar2SetTemplateSize2(g_ar2Handle, 6);
    }
    else
    {
        ARLOGi("Using NFT tracking settings for more than one CPU.\n");
        ar2SetTrackingThresh(g_ar2Handle, 5.0);
        ar2SetSimThresh(g_ar2Handle, 0.50);
        ar2SetSearchFeatureNum(g_ar2Handle, 16);
        ar2SetSearchSize(g_ar2Handle, 12);
        ar2SetTemplateSize1(g_ar2Handle, 6);
        ar2SetTemplateSize2(g_ar2Handle, 6);
    }

    // NFT dataset loading will happen later.
    return (TRUE);
}

// Modifies globals: threadHandle, surfaceSet[], surfaceSetCount
int UnloadNFTData(void)
{
    int i, j;

    if (g_threadHandle)
    {
        ARLOGi("Stopping NFT2 tracking thread.\n");
        TrackingInitQuit(&g_threadHandle);
    }

    j = 0;

    for (i = 0; i < g_surfaceSetCount; i++)
    {
        if (j == 0)
            ARLOGi("Unloading NFT tracking surfaces.\n");

        ar2FreeSurfaceSet(&g_surfaceSet[i]); // Also sets surfaceSet[i] to NULL.
        j++;
    }

    if (j > 0)
        ARLOGi("Unloaded %d NFT tracking surfaces.\n", j);

    g_surfaceSetCount = 0;

    return 0;
}

// References globals: markersNFTCount
// Modifies globals: threadHandle, surfaceSet[], surfaceSetCount, markersNFT[]
int LoadNFTData(void)
{
    int           i;
    KpmRefDataSet *refDataSet;

    // If data was already loaded, stop KPM tracking thread and unload previously loaded data.
    if (g_threadHandle)
    {
        ARLOGi("Reloading NFT data.\n");
        UnloadNFTData();
    }
    else
    {
        ARLOGi("Loading NFT data.\n");
    }

    refDataSet = NULL;

    for (i = 0; i < g_nMarkersNFTCount; i++)
    {
        // Load KPM data.
        KpmRefDataSet *refDataSet2;
        ARLOGi("Reading %s.fset3\n", g_pMarkersNFT[i].datasetPathname);
        if (kpmLoadRefDataSet(g_pMarkersNFT[i].datasetPathname, "fset3", &refDataSet2) < 0)
        {
            ARLOGe("Error reading KPM data from %s.fset3\n", g_pMarkersNFT[i].datasetPathname);
            g_pMarkersNFT[i].pageNo = -1;
            continue;
        }

        g_pMarkersNFT[i].pageNo = g_surfaceSetCount;
        ARLOGi("  Assigned page no. %d.\n", g_surfaceSetCount);
        if (kpmChangePageNoOfRefDataSet(refDataSet2, KpmChangePageNoAllPages, g_surfaceSetCount) < 0)
        {
            ARLOGe("Error: kpmChangePageNoOfRefDataSet\n");
            exit(-1);
        }

        if (kpmMergeRefDataSet(&refDataSet, &refDataSet2) < 0)
        {
            ARLOGe("Error: kpmMergeRefDataSet\n");
            exit(-1);
        }

        ARLOGi("  Done.\n");

        // Load AR2 data.
        ARLOGi("Reading %s.fset\n", g_pMarkersNFT[i].datasetPathname);

        if ((g_surfaceSet[g_surfaceSetCount] = ar2ReadSurfaceSet(g_pMarkersNFT[i].datasetPathname, "fset", NULL)) == NULL)
        {
            ARLOGe("Error reading data from %s.fset\n", g_pMarkersNFT[i].datasetPathname);
        }

        ARLOGi("  Done.\n");

        g_surfaceSetCount++;
        if (g_surfaceSetCount == PAGES_MAX)
            break;
    }

    if (kpmSetRefDataSet(g_kpmHandle, refDataSet) < 0)
    {
        ARLOGe("Error: kpmSetRefDataSet\n");
        exit(-1);
    }

    kpmDeleteRefDataSet(&refDataSet);

    // Start the KPM tracking thread.
    g_threadHandle = TrackingInitInit(g_kpmHandle);
    if (!g_threadHandle)
        exit(-1);

    ARLOGi("Loading of NFT data complete.\n");
    return (TRUE);
}

void DrawCube(float fSize, float x, float y, float z)
{
    // Color cube data.
    int           i;
    const GLfloat cube_vertices[8][3] =
    {
        /* +z */{ 0.5f, 0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f },
        /* -z */{ 0.5f, 0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f }
    };
    const GLubyte cube_vertex_colors[8][4] =
    {
        { 255, 255, 255, 255 }, { 255, 255, 0, 255 }, { 0, 255, 0, 255 }, { 0, 255, 255, 255 },
        { 255, 0, 255, 255 }, { 255, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 255, 255 }
    };
    const GLushort cube_faces[6][4] =    /* ccw-winding */
    { /* +z */{ 3, 2, 1, 0 }, /* -y */{ 2, 3, 7, 6 }, /* +y */{ 0, 1, 5, 4 },
    /* -x */{ 3, 0, 4, 7 }, /* +x */{ 1, 2, 6, 5 }, /* -z */{ 4, 5, 6, 7 } };

    glPushMatrix(); // Save world coordinate system.
    glTranslatef(x, y, z);
    glScalef(fSize, fSize, fSize);

#ifdef ANDROID
    glStateCacheDisableLighting();
    glStateCacheDisableTex2D();
    glStateCacheDisableBlend();
#else
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
#endif
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, cube_vertex_colors);
    glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    for (i = 0; i < 6; i++)
    {
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, &(cube_faces[i][0]));
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glColor4ub(0, 0, 0, 255);

    for (i = 0; i < 6; i++)
    {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, &(cube_faces[i][0]));
    }

    glPopMatrix();    // Restore world coordinate system.
}