/*
 *  nftSimple.c
 *  ARToolKit5
 *
 *  Demonstration of ARToolKit NFT. Render a color cube.
 *
 *  Press '?' while running for help on available key commands.
 *
 *  Disclaimer: IMPORTANT:  This Daqri software is supplied to you by Daqri
 *  LLC ("Daqri") in consideration of your agreement to the following
 *  terms, and your use, installation, modification or redistribution of
 *  this Daqri software constitutes acceptance of these terms.  If you do
 *  not agree with these terms, please do not use, install, modify or
 *  redistribute this Daqri software.
 *
 *  In consideration of your agreement to abide by the following terms, and
 *  subject to these terms, Daqri grants you a personal, non-exclusive
 *  license, under Daqri's copyrights in this original Daqri software (the
 *  "Daqri Software"), to use, reproduce, modify and redistribute the Daqri
 *  Software, with or without modifications, in source and/or binary forms;
 *  provided that if you redistribute the Daqri Software in its entirety and
 *  without modifications, you must retain this notice and the following
 *  text and disclaimers in all such redistributions of the Daqri Software.
 *  Neither the name, trademarks, service marks or logos of Daqri LLC may
 *  be used to endorse or promote products derived from the Daqri Software
 *  without specific prior written permission from Daqri.  Except as
 *  expressly stated in this notice, no other rights or licenses, express or
 *  implied, are granted by Daqri herein, including but not limited to any
 *  patent rights that may be infringed by your derivative works or by other
 *  works in which the Daqri Software may be incorporated.
 *
 *  The Daqri Software is provided by Daqri on an "AS IS" basis.  DAQRI
 *  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 *  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE, REGARDING THE DAQRI SOFTWARE OR ITS USE AND
 *  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *  IN NO EVENT SHALL DAQRI BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *  MODIFICATION AND/OR DISTRIBUTION OF THE DAQRI SOFTWARE, HOWEVER CAUSED
 *  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 *  STRICT LIABILITY OR OTHERWISE, EVEN IF DAQRI HAS BEEN ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Copyright 2015 Daqri LLC. All Rights Reserved.
 *  Copyright 2007-2015 ARToolworks, Inc. All Rights Reserved.
 *
 *  Author(s): Philip Lamb.
 *
 */


// ============================================================================
// Includes
// ============================================================================

#ifdef _WIN32
#  include <windows.h>
#endif
#include <stdio.h>
#ifdef _WIN32
#  define snprintf _snprintf
#endif
#include <string>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include <AR/gsub_lite.h>
#include <AR/arFilterTransMat.h>
#include <AR2/tracking.h>

#include "ARMarkerNFT.h"
#include "TrackingSub.h"
#include "NFTSimple.h"
#include "VirtualEnvironment.h"

// ============================================================================
// Constants
// ============================================================================

#define VIEW_SCALEFACTOR  1.0               // Units received from ARToolKit tracking will be multiplied by this factor before being used in OpenGL drawing.
#define VIEW_DISTANCE_MIN 10.0              // Objects closer to the camera than this will not be displayed. OpenGL units.
#define VIEW_DISTANCE_MAX 10000.0           // Objects further away from the camera than this will not be displayed. OpenGL units.

// ============================================================================
// Global variables
// ============================================================================

// Preferences.
static int prefWindowed = TRUE;
static int prefWidth    = 640;              // Fullscreen mode width.
static int prefHeight   = 480;              // Fullscreen mode height.
static int prefDepth    = 32;               // Fullscreen mode bit depth.
static int prefRefresh  = 0;                // Fullscreen mode refresh rate. Set to 0 to use default rate.

// Image acquisition.
static ARUint8 *gARTImage             = NULL;
static long    gCallCountMarkerDetect = 0;

// Drawing.
static int                       gWindowW;
static int                       gWindowH;
static ARParamLT                 *gCparamLT       = NULL;
static ARGL_CONTEXT_SETTINGS_REF gArglSettings    = NULL;
static int                       gDrawRotate      = FALSE;
static float                     gDrawRotateAngle = 0;  // For use in drawing.
static ARdouble                  cameraLens[16];
static ARdouble                  g_fCameraPose[16];
static int                       g_nCameraPoseValid;


// ============================================================================
// Function prototypes
// ============================================================================

static int SetupCamera(const char *cparam_name, char *vconf, ARParamLT **cparamLT_p);
static void Cleanup(void);
static void Keyboard(unsigned char key, int x, int y);
static void Visibility(int visible);
static void Reshape(int w, int h);
static void Display(void);

// ============================================================================
// Functions
// ============================================================================

int main(int argc, char **argv)
{
    char       glutGamemode[32];
    const char *cparam_name               = "Data2/camera_para_640x480.dat";
    const char markerConfigDataFilename[] = "Data2/pinball-markers.dat";
    const char objectDataFilename[]       = "OSG/axes.dat"; // "OSG/sterowiec-lwo.dat"; //"Data/cow.dat";

#ifdef WIN32
    char vconf[] = "-device=WinDS -showDialog  -flipV";
#else
    char vconf[] = "";
#endif

    // If using 1280x720, we should use 1280x720 camera parameter file.
    // const char *cparam_name = "Data2/camera_para_640x480.dat";
    // const char *cparam_name = "Data2/Logitech_camera_para_1280x720.dat";

    // char       vconf[]      = "-device=WinMF -format=BGRA -width=640 -height=480 -devNum=0";   // WinMF tracking is OK.
    // char       vconf[]      = "-device=WinHD";                        // WinHD cannot initialize camera.
    // char       vconf[]      = "-device=WinDF";                        // WinDF cannot initialize camera.
    // char       vconf[]      = "-device=WinDSVL";                      // WinDSVL only works in 640x480, but tracking doesn't work well.
    // char       vconf[]      = "-device=WinDS -showDialog  -flipV";    // WinDS tracks marker well.

    // const char markerConfigDataFilename[] = "Data2/markers.dat";

#ifdef DEBUG
    arLogLevel = AR_LOG_LEVEL_DEBUG;
#endif

    //
    // Library init.
    //

    glutInit(&argc, argv);

    //
    // Video setup.
    //

#ifdef _WIN32
    CoInitialize(NULL);
#endif

    if (!SetupCamera(cparam_name, vconf, &gCparamLT))
    {
        ARLOGe("main(): Unable to set up AR camera.\n");
        exit(-1);
    }

    //
    // AR init.
    //

    // Create the OpenGL projection from the calibrated camera parameters.
    arglCameraFrustumRH(&(gCparamLT->param), VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, cameraLens);
    g_nCameraPoseValid = FALSE;

    if (!InitNFT(gCparamLT, arVideoGetPixelFormat()))
    {
        ARLOGe("main(): Unable to init NFT.\n");
        exit(-1);
    }

    //
    // Graphics setup.
    //

    // Set up GL context(s) for OpenGL to draw into.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    if (!prefWindowed)
    {
        if (prefRefresh)
            sprintf(glutGamemode, "%ix%i:%i@%i", prefWidth, prefHeight, prefDepth, prefRefresh);
        else
            sprintf(glutGamemode, "%ix%i:%i", prefWidth, prefHeight, prefDepth);

        glutGameModeString(glutGamemode);
        glutEnterGameMode();
    }
    else
    {
        glutInitWindowSize(gCparamLT->param.xsize, gCparamLT->param.ysize);
        glutCreateWindow(argv[0]);
    }

    // Setup ARgsub_lite library for current OpenGL context.
    if ((gArglSettings = arglSetupForCurrentContext(&(gCparamLT->param), arVideoGetPixelFormat())) == NULL)
    {
        ARLOGe("main(): arglSetupForCurrentContext() returned error.\n");
        Cleanup();
        exit(-1);
    }

    // Load objects (i.e. OSG models).
    VirtualEnvironmentInit(objectDataFilename);
    VirtualEnvironmentHandleARViewUpdatedCameraLens(cameraLens);

    arUtilTimerReset();

    //
    // Markers setup.
    //

    // Load marker(s).
    NewMarkers(markerConfigDataFilename, &g_pMarkersNFT, &g_nMarkersNFTCount);
    if (!g_nMarkersNFTCount)
    {
        ARLOGe("Error loading markers from config. file '%s'.\n", markerConfigDataFilename);
        Cleanup();
        exit(-1);
    }

    ARLOGi("Marker count = %d\n", g_nMarkersNFTCount);

    // Marker data has been loaded, so now load NFT data.
    if (!LoadNFTData())
    {
        ARLOGe("Error loading NFT data.\n");
        Cleanup();
        exit(-1);
    }

    // Start the video.
    if (arVideoCapStart() != 0)
    {
        ARLOGe("setupCamera(): Unable to begin camera data capture.\n");
        return (FALSE);
    }

    // Register GLUT event-handling callbacks.
    // NB: mainLoop() is registered by Visibility.
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutVisibilityFunc(Visibility);
    glutKeyboardFunc(Keyboard);

    glutMainLoop();

    return (0);
}

static void DrawCubeUpdate(float timeDelta)
{
    if (gDrawRotate)
    {
        gDrawRotateAngle += timeDelta * 45.0f;         // Rotate cube at 45 degrees per second.
        if (gDrawRotateAngle > 360.0f)
            gDrawRotateAngle -= 360.0f;
    }
}

static int SetupCamera(const char *cparam_name, char *vconf, ARParamLT **cparamLT_p)
{
    ARParam         cparam;
    int             xsize, ysize;
    AR_PIXEL_FORMAT pixFormat;

    // Open the video path.
    if (arVideoOpen(vconf) < 0)
    {
        ARLOGe("setupCamera(): Unable to open connection to camera.\n");
        return (FALSE);
    }

    // Find the size of the window.
    if (arVideoGetSize(&xsize, &ysize) < 0)
    {
        ARLOGe("setupCamera(): Unable to determine camera frame size.\n");
        arVideoClose();
        return (FALSE);
    }

    ARLOGi("Camera image size (x,y) = (%d,%d)\n", xsize, ysize);

    // Get the format in which the camera is returning pixels.
    pixFormat = arVideoGetPixelFormat();
    if (pixFormat == AR_PIXEL_FORMAT_INVALID)
    {
        ARLOGe("setupCamera(): Camera is using unsupported pixel format.\n");
        arVideoClose();
        return (FALSE);
    }

    // Load the camera parameters, resize for the window and init.
    if (arParamLoad(cparam_name, 1, &cparam) < 0)
    {
        ARLOGe("setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        arVideoClose();
        return (FALSE);
    }

    if (cparam.xsize != xsize || cparam.ysize != ysize)
    {
        ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
        arParamChangeSize(&cparam, xsize, ysize, &cparam);
    }

#ifdef DEBUG
    ARLOG("*** Camera Parameter ***\n");
    arParamDisp(&cparam);
#endif
    if ((*cparamLT_p = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL)
    {
        ARLOGe("setupCamera(): Error: arParamLTCreate.\n");
        arVideoClose();
        return (FALSE);
    }

    return (TRUE);
}

static void Cleanup(void)
{
    VirtualEnvironmentFinal();

    DeleteMarkers(&g_pMarkersNFT, &g_nMarkersNFTCount);

    // NFT cleanup.
    UnloadNFTData();
    ARLOGd("Cleaning up ARToolKit NFT handles.\n");
    ar2DeleteHandle(&g_ar2Handle);
    kpmDeleteHandle(&g_kpmHandle);
    arParamLTFree(&gCparamLT);

    // OpenGL cleanup.
    arglCleanup(gArglSettings);
    gArglSettings = NULL;

    // Camera cleanup.
    arVideoCapStop();
    arVideoClose();

#ifdef _WIN32
    CoUninitialize();
#endif
}

static void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 0x1B: // Quit.
    case 'Q':
    case 'q':
        Cleanup();
        exit(0);
        break;

    case ' ':
        gDrawRotate = !gDrawRotate;
        break;

    case '?':
    case '/':
        ARLOG("Keys:\n");
        ARLOG(" q or [esc]    Quit demo.\n");
        ARLOG(" ? or /        Show this help.\n");
        ARLOG("\nAdditionally, the ARVideo library supplied the following help text:\n");
        arVideoDispOption();
        break;

    default:
        break;
    }
}

static void mainLoop(void)
{
    static int ms_prev;
    int        ms;
    float      s_elapsed;
    ARUint8    *image;

    // NFT results.
    static int   detectedPage = -2; // -2 Tracking not initialized, -1 tracking initialized OK, >= 0 tracking online on page.
    static float trackingTrans[3][4];

    int i, j, k;

    // Find out how long since mainLoop() last ran.
    ms        = glutGet(GLUT_ELAPSED_TIME);
    s_elapsed = (float)(ms - ms_prev) * 0.001f;
    if (s_elapsed < 0.01f)
        return;                        // Don't update more often than 100 Hz.

    ms_prev = ms;

    // Update drawing.
    DrawCubeUpdate(s_elapsed);

    // Grab a video frame.
    if ((image = arVideoGetImage()) != NULL)
    {
        gARTImage = image;              // Save the fetched image.
        gCallCountMarkerDetect++;       // Increment ARToolKit FPS counter.

        // Run marker detection on frame
        if (g_threadHandle)
        {
            // Perform NFT tracking.
            float err;
            int   ret;
            int   pageNo;

            if (detectedPage == -2)
            {
                TrackingInitStart(g_threadHandle, gARTImage);
                detectedPage = -1;
            }

            if (detectedPage == -1)
            {
                ret = TrackingInitGetResult(g_threadHandle, trackingTrans, &pageNo);
                if (ret == 1)
                {
                    if (pageNo >= 0 && pageNo < g_surfaceSetCount)
                    {
                        ARLOGd("Detected page %d.\n", pageNo);
                        detectedPage = pageNo;
                        ar2SetInitTrans(g_surfaceSet[detectedPage], trackingTrans);
                    }
                    else
                    {
                        ARLOGe("Detected bad page %d.\n", pageNo);
                        detectedPage = -2;
                    }
                }
                else if (ret < 0)
                {
                    ARLOGd("No page detected.\n");
                    detectedPage = -2;
                }
            }

            if (detectedPage >= 0 && detectedPage < g_surfaceSetCount)
            {
                if (ar2Tracking(g_ar2Handle, g_surfaceSet[detectedPage], gARTImage, trackingTrans, &err) < 0)
                {
                    ARLOGd("Tracking lost.\n");
                    detectedPage = -2;
                }
                else
                {
                    ARLOGd("Tracked page %d (max %d).\n", detectedPage, g_surfaceSetCount - 1);
                }
            }
        }
        else
        {
            ARLOGe("Error: threadHandle\n");
            detectedPage = -2;
        }

        // Update markers.
        for (i = 0; i < g_nMarkersNFTCount; i++)
        {
            g_pMarkersNFT[i].validPrev = g_pMarkersNFT[i].valid;
            if (g_pMarkersNFT[i].pageNo >= 0 && g_pMarkersNFT[i].pageNo == detectedPage)
            {
                g_pMarkersNFT[i].valid = TRUE;

                for (j = 0; j < 3; j++)
                    for (k = 0; k < 4; k++)
                        g_pMarkersNFT[i].trans[j][k] = trackingTrans[j][k];
            }
            else
                g_pMarkersNFT[i].valid = FALSE;

            if (g_pMarkersNFT[i].valid)
            {
                // Filter the pose estimate.
                if (g_pMarkersNFT[i].ftmi)
                {
                    if (arFilterTransMat(g_pMarkersNFT[i].ftmi, g_pMarkersNFT[i].trans, !g_pMarkersNFT[i].validPrev) < 0)
                    {
                        ARLOGe("arFilterTransMat error with marker %d.\n", i);
                    }
                }

                if (!g_pMarkersNFT[i].validPrev)
                {
                    // Marker has become visible, tell any dependent objects.
                    // --->
                    VirtualEnvironmentHandleARMarkerAppeared(i);
                }

                // We have a new pose, so set that.
                arglCameraViewRH((const ARdouble(*)[4])g_pMarkersNFT[i].trans, g_pMarkersNFT[i].pose.T, VIEW_SCALEFACTOR);

                // Tell any dependent objects about the update.
                // --->
                VirtualEnvironmentHandleARMarkerWasUpdated(i, g_pMarkersNFT[i].pose);
            }
            else
            {
                if (g_pMarkersNFT[i].validPrev)
                {
                    // Marker has ceased to be visible, tell any dependent objects.
                    // --->
                    VirtualEnvironmentHandleARMarkerDisappeared(i);
                }
            }
        }

        // Tell GLUT the display has changed.
        glutPostRedisplay();
    }
}

//
// This function is called on events when the visibility of the
// GLUT window changes (including when it first becomes visible).
//
static void Visibility(int visible)
{
    if (visible == GLUT_VISIBLE)
    {
        glutIdleFunc(mainLoop);
    }
    else
    {
        glutIdleFunc(NULL);
    }
}

//
// This function is called when the
// GLUT window is resized.
//
static void Reshape(int w, int h)
{
    GLint viewport[4];

    gWindowW = w;
    gWindowH = h;

    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = w;
    viewport[3] = h;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // Call through to anyone else who needs to know about window sizing here.
    VirtualEnvironmentHandleARViewUpdatedViewport(viewport);
}

//
// This function is called when the window needs redrawing.
//
static void Display(void)
{
    int i;

    // Select correct buffer for this context.
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the buffers for new frame.

    arglPixelBufferDataUpload(gArglSettings, gARTImage);
    arglDispImage(gArglSettings);
    gARTImage = NULL;     // Invalidate image data.

    // Set up 3D mode.
    glMatrixMode(GL_PROJECTION);
#ifdef ARDOUBLE_IS_FLOAT
    glLoadMatrixf(cameraLens);
#else
    glLoadMatrixd(cameraLens);
#endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    // Set any initial per-frame GL state you require here.
    // --->

    // Lighting and geometry that moves with the camera should be added here.
    // (I.e. should be specified before marker pose transform.)
    // --->

    VirtualEnvironmentHandleARViewDrawPreCamera();

    if (g_nCameraPoseValid)
    {
#ifdef ARDOUBLE_IS_FLOAT
        glMultMatrixf(g_fCameraPose);
#else
        glMultMatrixd(g_fCameraPose);
#endif

        // All lighting and geometry to be drawn in world coordinates goes here.
        // --->
        VirtualEnvironmentHandleARViewDrawPostCamera();
    }

    for (i = 0; i < g_nMarkersNFTCount; i++)
    {
        if (g_pMarkersNFT[i].valid)
        {
#ifdef ARDOUBLE_IS_FLOAT
            glLoadMatrixf(g_pMarkersNFT[i].pose.T);
#else
            glLoadMatrixd(g_pMarkersNFT[i].pose.T);
#endif
            // All lighting and geometry to be drawn relative to the marker goes here.
            // --->

            // Benet-add for test
            // glScalef(15.0f, 20.0f, 4.0f);
            // glTranslatef(20.0, 20.0f, 0.0f);

            DrawCube(40.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    // Set up 2D mode.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (GLdouble)gWindowW, 0, (GLdouble)gWindowH, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Add your own 2D overlays here.
    // --->

    glutSwapBuffers();
}