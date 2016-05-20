/*
 *  nftSimple.cpp
 *  ARToolKit for Android
 *
 *  An NFT example with all ARToolKit setup performed in native code,
 *  and with basic OpenGL ES rendering of a color cube.
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
 *  Copyright 2011-2015 ARToolworks, Inc. All Rights Reserved.
 *
 *  Author(s): Philip Lamb
 *
 */

// ============================================================================
// Includes
// ============================================================================

#include <jni.h>
#include <android/log.h>
#include <stdlib.h> // malloc()

#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include <AR/gsub_es.h>
#include <AR/arFilterTransMat.h>
#include <AR2/tracking.h>

#include "ARMarkerNFT.h"
#include "TrackingSub.h"
#include "NFTSimple.h"
#include "VirtualEnvironment.h"
#include "osgPlugins.h"

// ============================================================================
// Types
// ============================================================================

typedef enum
{
    ARViewContentModeScaleToFill,
    ARViewContentModeScaleAspectFit,      // contents scaled to fit with fixed aspect. remainder is transparent
    ARViewContentModeScaleAspectFill,     // contents scaled to fill with fixed aspect. some portion of content may be clipped.
    ARViewContentModeCenter,              // contents remain same size. positioned adjusted.
    ARViewContentModeTop,
    ARViewContentModeBottom,
    ARViewContentModeLeft,
    ARViewContentModeRight,
    ARViewContentModeTopLeft,
    ARViewContentModeTopRight,
    ARViewContentModeBottomLeft,
    ARViewContentModeBottomRight,
    // ARViewContentModeRedraw,              // redraw on bounds change
} ARViewContentMode;

enum viewPortIndices
{
    viewPortIndexLeft = 0,
    viewPortIndexBottom,
    viewPortIndexWidth,
    viewPortIndexHeight
};

// ============================================================================
// Constants
// ============================================================================

#ifndef MAX
#  define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif
#ifndef MIN
#  define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

// Logging macros
#define  LOG_TAG "NFTSimpleNative"
#ifdef DEBUG
#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define  LOGD(...)
#endif
#define  LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============================================================================
// Function prototypes.
// ============================================================================

// Utility preprocessor directive so only one change needed if Java class name changes
#define JNIFUNCTION_NATIVE(sig) Java_com_tencent_NFTSimple_nftSimpleActivity_ ## sig

extern "C" {
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeCreate(JNIEnv * env, jobject object, jobject instanceOfAndroidContext));
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeStart(JNIEnv * env, jobject object));
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeStop(JNIEnv * env, jobject object));
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeDestroy(JNIEnv * env, jobject object));
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeVideoInit(JNIEnv * env, jobject object, jint w, jint h, jint cameraIndex, jboolean cameraIsFrontFacing));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeVideoFrame(JNIEnv * env, jobject obj, jbyteArray pinArray));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeSurfaceCreated(JNIEnv * env, jobject object));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeSurfaceChanged(JNIEnv * env, jobject object, jint w, jint h));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeDisplayParametersChanged(JNIEnv * env, jobject object, jint orientation, jint width, jint height, jint dpi));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeDrawFrame(JNIEnv * env, jobject obj));
JNIEXPORT void JNICALL     JNIFUNCTION_NATIVE(nativeSetInternetState(JNIEnv * env, jobject obj, jint state));
};

static void nativeVideoGetCparamCallback(const ARParam *cparam, void *userdata);
static void* LoadNFTDataAsync(THREAD_HANDLE_T *threadHandle);

// ============================================================================
// Global variables
// ============================================================================

// Preferences.
static const char *g_cparaName              = "Data/camera_para.dat";   ///< Camera parameters file
static const char *markerConfigDataFilename = "Data/markers.dat";
static const char *objectDataFilename       = "OSG/p51d-jw-animated.dat";

// Image acquisition.
static AR2VideoParamT  *g_Vid        = NULL;
static bool            g_videoInited = false;                           ///< true when ready to receive video frames.
static int             g_videoWidth  = 0;                               ///< Width of the video frame in pixels.
static int             g_videoHeight = 0;                               ///< Height of the video frame in pixels.
static AR_PIXEL_FORMAT gPixFormat;                                      ///< Pixel format from ARToolKit enumeration.
static ARUint8         *gVideoFrame                         = NULL;     ///< Buffer containing current video frame.
static size_t          gVideoFrameSize                      = 0;        ///< Size of buffer containing current video frame.
static bool            videoFrameNeedsPixelBufferDataUpload = false;
static int             gCameraIndex                         = 0;
static bool            gCameraIsFrontFacing                 = false;

// NFT.
static THREAD_HANDLE_T *nftDataLoadingThreadHandle = NULL;
static int             nftDataLoaded               = false;

// NFT results.
static int   detectedPage = -2; // -2 Tracking not initialized, -1 tracking initialized OK, >= 0 tracking online on page.
static float trackingTrans[3][4];

// Drawing.
static int                       backingWidth;
static int                       backingHeight;
static GLint                     viewPort[4];
static ARViewContentMode         gContentMode          = ARViewContentModeScaleAspectFill;
static bool                      gARViewLayoutRequired = false;
static ARParamLT                 *gCparamLT            = NULL;      ///< Camera parameters
static ARGL_CONTEXT_SETTINGS_REF gArglSettings         = NULL;      ///< GL settings for rendering video background
static const ARdouble            NEAR_PLANE            = 10.0f;     ///< Near plane distance for projection matrix calculation
static const ARdouble            FAR_PLANE             = 5000.0f;   ///< Far plane distance for projection matrix calculation
static ARdouble                  cameraLens[16];
static ARdouble                  cameraPose[16];
static int                       cameraPoseValid;
static bool                      gARViewInited = false;

// Drawing orientation.
static int gDisplayOrientation = 1; // range [0-3]. 1=landscape.
static int gDisplayWidth       = 0;
static int gDisplayHeight      = 0;
static int gDisplayDPI         = 160; // Android default.

static bool gContentRotate90 = false;
static bool gContentFlipV    = false;
static bool gContentFlipH    = false;

// Network.
static int gInternetState = -1;


// ============================================================================
// Functions
// ============================================================================

//
// Life-cycle functions.
//

JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeCreate(JNIEnv * env, jobject object, jobject instanceOfAndroidContext))
{
    int err_i;

    LOGD("nativeCreate\n");

    // Change working directory for the native process, so relative paths can be used for file access.
    arUtilChangeToResourcesDirectory(AR_UTIL_RESOURCES_DIRECTORY_BEHAVIOR_BEST, NULL, instanceOfAndroidContext);

    // Load marker(s).
    NewMarkers(markerConfigDataFilename, &g_pMarkersNFT, &g_nMarkersNFTCount);
    if (!g_nMarkersNFTCount)
    {
        LOGE("Error loading markers from config. file '%s'.", markerConfigDataFilename);
        return false;
    }

    LOGD("Marker count = %d\n", g_nMarkersNFTCount);

    return (true);
}

JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeStart(JNIEnv * env, jobject object))
{
    LOGD("nativeStart\n");

    g_Vid = ar2VideoOpen("");
    if (!g_Vid)
    {
        LOGE("Error: ar2VideoOpen.\n");
        return (false);
    }

    // Since most NFT init can't be completed until the video frame size is known,
    // and NFT surface loading depends on NFT init, all that will be deferred.

    // Also, VirtualEnvironment init depends on having an OpenGL context, and so that also
    // forces us to defer VirtualEnvironment init.

    // ARGL init depends on both these things, which forces us to defer it until the
    // main frame loop.

    return (true);
}

// cleanup();
JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeStop(JNIEnv * env, jobject object))
{
    int i;
    int j;

    LOGD("nativeStop\n");

    // Can't call arglCleanup() here, because nativeStop is not called on rendering thread.

    // NFT cleanup.
    if (g_threadHandle)
    {
        LOGD("Stopping NFT2 tracking thread.");
        TrackingInitQuit(&g_threadHandle);
        detectedPage = -2;
    }

    j = 0;

    for (i = 0; i < g_surfaceSetCount; i++)
    {
        if (g_surfaceSet[i])
        {
#ifdef DEBUG
            if (j == 0)
                LOGD("Unloading NFT tracking surfaces.");
#endif
            ar2FreeSurfaceSet(&g_surfaceSet[i]); // Sets surfaceSet[i] to NULL.
            j++;
        }
    }

#ifdef DEBUG
    if (j > 0)
        LOGD("Unloaded %d NFT tracking surfaces.", j);
#endif

    g_surfaceSetCount = 0;
    nftDataLoaded     = false;

    LOGD("Cleaning up ARToolKit NFT handles.");

    ar2DeleteHandle(&g_ar2Handle);
    kpmDeleteHandle(&g_kpmHandle);
    arParamLTFree(&gCparamLT);

    // OpenGL cleanup -- not done here.

    // Video cleanup.
    if (gVideoFrame)
    {
        free(gVideoFrame);
        gVideoFrame     = NULL;
        gVideoFrameSize = 0;
    }

    ar2VideoClose(g_Vid);
    g_Vid         = NULL;
    g_videoInited = false;

    return (true);
}

JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeDestroy(JNIEnv * env, jobject object))
{
    LOGD("nativeDestroy\n");

    VirtualEnvironmentFinal();

    DeleteMarkers(&g_pMarkersNFT, &g_nMarkersNFTCount);

    return (true);
}

//
// Camera functions.
//

JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeVideoInit(JNIEnv * env, jobject object, jint w, jint h, jint cameraIndex, jboolean cameraIsFrontFacing))
{
    LOGD("nativeVideoInit\n");

    // As of ARToolKit v5.0, NV21 format video frames are handled natively,
    // and no longer require color conversion to RGBA. A buffer (gVideoFrame)
    // must be set aside to copy the frame from the Java side.
    // If you still require RGBA format information from the video,
    // you can create your own additional buffer, and then unpack the NV21
    // frames into it in nativeVideoFrame() below.
    // Here is where you'd allocate the buffer:
    // ARUint8 *myRGBABuffer = (ARUint8 *)malloc(videoWidth * videoHeight * 4);
    gPixFormat      = AR_PIXEL_FORMAT_NV21;
    gVideoFrameSize = (sizeof(ARUint8) * (w * h + 2 * w / 2 * h / 2));
    gVideoFrame     = (ARUint8*) (malloc(gVideoFrameSize));
    if (!gVideoFrame)
    {
        gVideoFrameSize = 0;
        LOGE("Error allocating frame buffer");
        return false;
    }

    g_videoWidth         = w;
    g_videoHeight        = h;
    gCameraIndex         = cameraIndex;
    gCameraIsFrontFacing = cameraIsFrontFacing;
    LOGI("Video camera %d (%s), %dx%d format %s, %d-byte buffer.",
         gCameraIndex, (gCameraIsFrontFacing ? "front" : "rear"), w, h, arUtilGetPixelFormatName(gPixFormat), gVideoFrameSize);

    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_WIDTH, g_videoWidth);
    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_HEIGHT, g_videoHeight);
    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_PIXELFORMAT, (int)gPixFormat);
    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_CAMERA_INDEX, gCameraIndex);
    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_CAMERA_FACE, gCameraIsFrontFacing);
    ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_INTERNET_STATE, gInternetState);

    if (ar2VideoGetCParamAsync(g_Vid, nativeVideoGetCparamCallback, NULL) < 0)
    {
        LOGE("Error getting cparam.\n");
        nativeVideoGetCparamCallback(NULL, NULL);
    }

    return (true);
}

static void nativeVideoGetCparamCallback(const ARParam *cparam_p, void *userdata)
{
    // Load the camera parameters, resize for the window and init.
    ARParam cparam;

    if (cparam_p)
        cparam = *cparam_p;
    else
    {
        LOGE("Unable to automatically determine camera parameters. Using default.\n");
        if (arParamLoad(g_cparaName, 1, &cparam) < 0)
        {
            LOGE("Error: Unable to load parameter file %s for camera.\n", g_cparaName);
            return;
        }
    }

    if (cparam.xsize != g_videoWidth || cparam.ysize != g_videoHeight)
    {
        LOGW("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
        arParamChangeSize(&cparam, g_videoWidth, g_videoHeight, &cparam);
    }

#ifdef DEBUG
    LOGD("*** Camera Parameter ***\n");
    arParamDisp(&cparam);
#endif

    if ((gCparamLT = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL)
    {
        LOGE("Error: arParamLTCreate.\n");
        return;
    }

    g_videoInited = true;

    //
    // AR init.
    //

    // Create the OpenGL projection from the calibrated camera parameters.
    arglCameraFrustumRHf(&gCparamLT->param, NEAR_PLANE, FAR_PLANE, cameraLens);
    cameraPoseValid = FALSE;

    if (!InitNFT(gCparamLT, gPixFormat))
    {
        LOGE("Error initialising NFT.\n");
        arParamLTFree(&gCparamLT);
        return;
    }

    // Marker data has already been loaded, so now load NFT data on a second thread.
    nftDataLoadingThreadHandle = threadInit(0, NULL, LoadNFTDataAsync);
    if (!nftDataLoadingThreadHandle)
    {
        LOGE("Error starting NFT loading thread.\n");
        arParamLTFree(&gCparamLT);
        return;
    }

    threadStartSignal(nftDataLoadingThreadHandle);
}

// References globals: markersNFTCount
// Modifies globals: trackingThreadHandle, surfaceSet[], surfaceSetCount, markersNFT[], markersNFTCount
static void* LoadNFTDataAsync(THREAD_HANDLE_T *threadHandle)
{
    int           i, j;
    KpmRefDataSet *refDataSet;

    while (threadStartWait(threadHandle) == 0)
    {
        LOGD("Loading NFT data.\n");

        // If data was already loaded, stop KPM tracking thread and unload previously loaded data.
        if (g_threadHandle)
        {
            LOGE("NFT2 tracking thread is running. Stopping it first.\n");
            TrackingInitQuit(&g_threadHandle);
            detectedPage = -2;
        }

        j = 0;

        for (i = 0; i < g_surfaceSetCount; i++)
        {
            if (j == 0)
                LOGE("Unloading NFT tracking surfaces.");

            ar2FreeSurfaceSet(&g_surfaceSet[i]); // Also sets surfaceSet[i] to NULL.
            j++;
        }

        if (j > 0)
            LOGE("Unloaded %d NFT tracking surfaces.\n", j);

        g_surfaceSetCount = 0;

        refDataSet = NULL;

        for (i = 0; i < g_nMarkersNFTCount; i++)
        {
            // Load KPM data.
            KpmRefDataSet *refDataSet2;
            LOGI("Reading %s.fset3\n", g_pMarkersNFT[i].datasetPathname);
            if (kpmLoadRefDataSet(g_pMarkersNFT[i].datasetPathname, "fset3", &refDataSet2) < 0)
            {
                LOGE("Error reading KPM data from %s.fset3\n", g_pMarkersNFT[i].datasetPathname);
                g_pMarkersNFT[i].pageNo = -1;
                continue;
            }

            g_pMarkersNFT[i].pageNo = g_surfaceSetCount;
            LOGI("  Assigned page no. %d.\n", g_surfaceSetCount);
            if (kpmChangePageNoOfRefDataSet(refDataSet2, KpmChangePageNoAllPages, g_surfaceSetCount) < 0)
            {
                LOGE("Error: kpmChangePageNoOfRefDataSet\n");
                exit(-1);
            }

            if (kpmMergeRefDataSet(&refDataSet, &refDataSet2) < 0)
            {
                LOGE("Error: kpmMergeRefDataSet\n");
                exit(-1);
            }

            LOGI("  Done.\n");

            // Load AR2 data.
            LOGI("Reading %s.fset\n", g_pMarkersNFT[i].datasetPathname);

            if ((g_surfaceSet[g_surfaceSetCount] = ar2ReadSurfaceSet(g_pMarkersNFT[i].datasetPathname, "fset", NULL)) == NULL)
            {
                LOGE("Error reading data from %s.fset\n", g_pMarkersNFT[i].datasetPathname);
            }

            LOGI("  Done.\n");

            g_surfaceSetCount++;
            if (g_surfaceSetCount == PAGES_MAX)
                break;
        }

        if (kpmSetRefDataSet(g_kpmHandle, refDataSet) < 0)
        {
            LOGE("Error: kpmSetRefDataSet");
            exit(-1);
        }

        kpmDeleteRefDataSet(&refDataSet);

        // Start the KPM tracking thread.
        g_threadHandle = TrackingInitInit(g_kpmHandle);
        if (!g_threadHandle)
            exit(-1);

        LOGD("Loading of NFT data complete.");

        threadEndSignal(threadHandle); // Signal that we're done.
    }

    return (NULL); // Exit this thread.
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeVideoFrame(JNIEnv * env, jobject obj, jbyteArray pinArray))
{
    int   i, j, k;
    jbyte *inArray;

    if (!g_videoInited)
    {
        LOGD("nativeVideoFrame !VIDEO\n");
        return; // No point in trying to track until video is initialized.
    }

    if (!nftDataLoaded)
    {
        if (!nftDataLoadingThreadHandle || threadGetStatus(nftDataLoadingThreadHandle) < 1)
        {
            LOGD("nativeVideoFrame !NFTDATA\n");
            return;
        }
        else
        {
            nftDataLoaded = true;
            threadWaitQuit(nftDataLoadingThreadHandle);
            threadFree(&nftDataLoadingThreadHandle); // Clean up.
        }
    }

    if (!gARViewInited)
    {
        LOGD("nativeVideoFrame !ARVIEW\n");
        return; // Also, we won't track until the ARView has been initialized.
    }

    LOGD("nativeVideoFrame\n");

    // Copy the incoming  YUV420 image in pinArray.
    env->GetByteArrayRegion(pinArray, 0, gVideoFrameSize, (jbyte*)gVideoFrame);

    // As of ARToolKit v5.0, NV21 format video frames are handled natively,
    // and no longer require color conversion to RGBA.
    // If you still require RGBA format information from the video,
    // here is where you'd do the conversion:
    // color_convert_common(gVideoFrame, gVideoFrame + videoWidth*videoHeight, videoWidth, videoHeight, myRGBABuffer);

    videoFrameNeedsPixelBufferDataUpload = true; // Note that buffer needs uploading. (Upload must be done on OpenGL context's thread.)

    // Run marker detection on frame
    if (g_threadHandle)
    {
        // Perform NFT tracking.
        float err;
        int   ret;
        int   pageNo;

        if (detectedPage == -2)
        {
            TrackingInitStart(g_threadHandle, gVideoFrame);
            detectedPage = -1;
        }

        if (detectedPage == -1)
        {
            ret = TrackingInitGetResult(g_threadHandle, trackingTrans, &pageNo);
            if (ret == 1)
            {
                if (pageNo >= 0 && pageNo < g_surfaceSetCount)
                {
                    LOGD("Detected page %d.\n", pageNo);

                    detectedPage = pageNo;
                    ar2SetInitTrans(g_surfaceSet[detectedPage], trackingTrans);
                }
                else
                {
                    LOGE("Detected bad page %d.\n", pageNo);
                    detectedPage = -2;
                }
            }
            else if (ret < 0)
            {
                LOGD("No page detected.\n");
                detectedPage = -2;
            }
        }

        if (detectedPage >= 0 && detectedPage < g_surfaceSetCount)
        {
            if (ar2Tracking(g_ar2Handle, g_surfaceSet[detectedPage], gVideoFrame, trackingTrans, &err) < 0)
            {
                LOGD("Tracking lost.\n");
                detectedPage = -2;
            }
            else
            {
                LOGD("Tracked page %d (max %d).\n", detectedPage, g_surfaceSetCount - 1);
            }
        }
    }
    else
    {
        LOGE("Error: trackingThreadHandle\n");
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
                    LOGE("arFilterTransMat error with marker %d.\n", i);
                }
            }

            if (!g_pMarkersNFT[i].validPrev)
            {
                // Marker has become visible, tell any dependent objects.
                VirtualEnvironmentHandleARMarkerAppeared(i);
            }

            // We have a new pose, so set that.
            arglCameraViewRHf(g_pMarkersNFT[i].trans, g_pMarkersNFT[i].pose.T, 1.0f /*VIEW_SCALEFACTOR*/);

            // Tell any dependent objects about the update.
            VirtualEnvironmentHandleARMarkerWasUpdated(i, g_pMarkersNFT[i].pose);
        }
        else
        {
            if (g_pMarkersNFT[i].validPrev)
            {
                // Marker has ceased to be visible, tell any dependent objects.
                VirtualEnvironmentHandleARMarkerDisappeared(i);
            }
        }
    }
}

//
// OpenGL functions.
//

//
// This is called whenever the OpenGL context has just been created or re-created.
// Note that GLSurfaceView is a bit asymmetrical here; we don't get a call when the
// OpenGL context is about to be deleted, it's just whipped out from under us. So it's
// possible that when we enter this function, we're actually resuming after such an
// event. What about resources we allocated previously which we didn't get time to
// free? Well, we don't have to worry about the OpenGL resources themselves, they
// were deleted along with the context. But, we should clean up any data structures we
// allocated with malloc etc. ARGL's settings falls into this category.
//
JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeSurfaceCreated(JNIEnv * env, jobject object))
{
    LOGD("nativeSurfaceCreated\n");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glStateCacheFlush();     // Make sure we don't hold outdated OpenGL state.

    if (gArglSettings)
    {
        arglCleanup(gArglSettings);         // Clean up any left-over ARGL data.
        gArglSettings = NULL;
    }

    gARViewInited = false;
}

//
// This is called when something about the surface changes. e.g. size.
//
// Modifies globals: backingWidth, backingHeight, gARViewLayoutRequired.
JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeSurfaceChanged(JNIEnv * env, jobject object, jint w, jint h))
{
    backingWidth  = w;
    backingHeight = h;

    LOGD("nativeSurfaceChanged backingWidth=%d, backingHeight=%d\n", w, h);

    // Call through to anyone else who needs to know about window sizing here.

    // In order to do something meaningful with the surface backing size in an AR sense,
    // we also need the content size, which we aren't guaranteed to have yet, so defer
    // the viewPort calculations.
    gARViewLayoutRequired = true;
}

// 0 = portrait, 1 = landscape (device rotated 90 degrees ccw), 2 = portrait upside down, 3 = landscape reverse (device rotated 90 degrees cw).
JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeDisplayParametersChanged(JNIEnv * env, jobject object, jint orientation, jint width, jint height, jint dpi))
{
    LOGD("nativeDisplayParametersChanged orientation=%d, size=%dx%d@%dpi\n", orientation, width, height, dpi);

    gDisplayOrientation = orientation;
    gDisplayWidth       = width;
    gDisplayHeight      = height;
    gDisplayDPI         = dpi;

    gARViewLayoutRequired = true;
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeSetInternetState(JNIEnv * env, jobject obj, jint state))
{
    gInternetState = state;
    if (g_Vid)
    {
        ar2VideoSetParami(g_Vid, AR_VIDEO_PARAM_ANDROID_INTERNET_STATE, state);
    }
}

// Lays out the AR view. Requires both video and OpenGL to be initialized, and must be called on OpenGL thread.
// References globals: gContentMode, backingWidth, backingHeight, videoWidth, videoHeight, .
// Modifies globals: gContentFlipV, gContentFlipH, gContentRotate90, viewPort, gARViewLayoutRequired.
static bool layoutARView(void)
{
    if (gDisplayOrientation == 0)
    {
        gContentRotate90 = true;
        gContentFlipV    = false;
        gContentFlipH    = gCameraIsFrontFacing;
    }
    else if (gDisplayOrientation == 1)
    {
        gContentRotate90 = false;
        gContentFlipV    = false;
        gContentFlipH    = gCameraIsFrontFacing;
    }
    else if (gDisplayOrientation == 2)
    {
        gContentRotate90 = true;
        gContentFlipV    = true;
        gContentFlipH    = (!gCameraIsFrontFacing);
    }
    else if (gDisplayOrientation == 3)
    {
        gContentRotate90 = false;
        gContentFlipV    = true;
        gContentFlipH    = (!gCameraIsFrontFacing);
    }

    arglSetRotate90(gArglSettings, gContentRotate90);
    arglSetFlipV(gArglSettings, gContentFlipV);
    arglSetFlipH(gArglSettings, gContentFlipH);

    // Calculate viewPort.
    int left, bottom, w, h;
    int contentWidth  = g_videoWidth;
    int contentHeight = g_videoHeight;

    if (gContentMode == ARViewContentModeScaleToFill)
    {
        w = backingWidth;
        h = backingHeight;
    }
    else
    {
        int contentWidthFinalOrientation  = (gContentRotate90 ? contentHeight : contentWidth);
        int contentHeightFinalOrientation = (gContentRotate90 ? contentWidth : contentHeight);
        if (gContentMode == ARViewContentModeScaleAspectFit || gContentMode == ARViewContentModeScaleAspectFill)
        {
            float scaleRatioWidth, scaleRatioHeight, scaleRatio;
            scaleRatioWidth  = (float)backingWidth / (float)contentWidthFinalOrientation;
            scaleRatioHeight = (float)backingHeight / (float)contentHeightFinalOrientation;
            if (gContentMode == ARViewContentModeScaleAspectFill)
                scaleRatio = MAX(scaleRatioHeight, scaleRatioWidth);
            else
                scaleRatio = MIN(scaleRatioHeight, scaleRatioWidth);

            w = (int)((float)contentWidthFinalOrientation * scaleRatio);
            h = (int)((float)contentHeightFinalOrientation * scaleRatio);
        }
        else
        {
            w = contentWidthFinalOrientation;
            h = contentHeightFinalOrientation;
        }
    }

    if (gContentMode == ARViewContentModeTopLeft
        || gContentMode == ARViewContentModeLeft
        || gContentMode == ARViewContentModeBottomLeft)
        left = 0;
    else if (gContentMode == ARViewContentModeTopRight
             || gContentMode == ARViewContentModeRight
             || gContentMode == ARViewContentModeBottomRight)
        left = backingWidth - w;
    else
        left = (backingWidth - w) / 2;

    if (gContentMode == ARViewContentModeBottomLeft
        || gContentMode == ARViewContentModeBottom
        || gContentMode == ARViewContentModeBottomRight)
        bottom = 0;
    else if (gContentMode == ARViewContentModeTopLeft
             || gContentMode == ARViewContentModeTop
             || gContentMode == ARViewContentModeTopRight)
        bottom = backingHeight - h;
    else
        bottom = (backingHeight - h) / 2;

    glViewport(left, bottom, w, h);

    viewPort[viewPortIndexLeft]   = left;
    viewPort[viewPortIndexBottom] = bottom;
    viewPort[viewPortIndexWidth]  = w;
    viewPort[viewPortIndexHeight] = h;

    LOGD("Viewport={%d, %d, %d, %d}\n", left, bottom, w, h);

    // Call through to anyone else who needs to know about changes in the ARView layout here.
    VirtualEnvironmentHandleARViewUpdatedViewport(viewPort);

    gARViewLayoutRequired = false;

    return (true);
}


// All tasks which require both video and OpenGL to be initialized should be performed here.
// References globals: gCparamLT, gPixFormat
// Modifies globals: gArglSettings
static bool InitARView(void)
{
    LOGD("Initialising ARView\n");

    if (gARViewInited)
        return (false);

    LOGD("Setting up argl.\n");

    if ((gArglSettings = arglSetupForCurrentContext(&gCparamLT->param, gPixFormat)) == NULL)
    {
        LOGE("Unable to setup argl.\n");
        return (false);
    }

    // Load objects (i.e. OSG models).
    VirtualEnvironmentInit(objectDataFilename);
    VirtualEnvironmentHandleARViewUpdatedCameraLens(cameraLens);

    LOGD("argl setup OK.\n");

    gARViewInited = true;

    LOGD("ARView initialised.\n");

    return (true);
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeDrawFrame(JNIEnv * env, jobject obj))
{
    float width, height;

    if (!g_videoInited)
    {
        LOGD("nativeDrawFrame !VIDEO\n");
        return; // No point in trying to draw until video is initialized.
    }

    LOGD("nativeDrawFrame\n");

    if (!gARViewInited)
    {
        if (!InitARView())
            return;
    }

    if (gARViewLayoutRequired)
        layoutARView();

    // Upload new video frame if required.
    if (videoFrameNeedsPixelBufferDataUpload)
    {
        arglPixelBufferDataUploadBiPlanar(gArglSettings, gVideoFrame, gVideoFrame + g_videoWidth * g_videoHeight);
        videoFrameNeedsPixelBufferDataUpload = false;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the buffers for new frame.

    // Display the current frame
    arglDispImage(gArglSettings);

    // Set up 3D mode.
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(cameraLens);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glStateCacheEnableDepthTest();

    // Set any initial per-frame GL state you require here.
    // --->

    // Lighting and geometry that moves with the camera should be added here.
    // (I.e. should be specified before camera pose transform.)
    // --->
    VirtualEnvironmentHandleARViewDrawPreCamera();

    // Draw an object on all valid markers.
    for (int i = 0; i < g_nMarkersNFTCount; i++)
    {
        if (g_pMarkersNFT[i].valid)
        {
            // benet-del: don't draw cube.
            // glLoadMatrixf(g_pMarkersNFT[i].pose.T);
            // DrawCube(40.0f, 0.0f, 0.0f, 20.0f);
        }
    }

    if (cameraPoseValid)
    {
        glMultMatrixf(cameraPose);

        // All lighting and geometry to be drawn in world coordinates goes here.
        // --->
        VirtualEnvironmentHandleARViewDrawPostCamera();
    }

    // If you added external OpenGL code above, and that code doesn't use the glStateCache routines,
    // then uncomment the line below.
    // glStateCacheFlush();

    // Set up 2D mode.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width  = (float)viewPort[viewPortIndexWidth];
    height = (float)viewPort[viewPortIndexHeight];
    glOrthof(0.0f, width, 0.0f, height, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glStateCacheDisableDepthTest();

    // Add your own 2D overlays here.
    // --->

    // If you added external OpenGL code above, and that code doesn't use the glStateCache routines,
    // then uncomment the line below.
    // glStateCacheFlush();

#ifdef DEBUG
    // Example of 2D drawing. It just draws a white border line. Change the 0 to 1 to enable.
    const GLfloat square_vertices[4][2] =
    {
        {0.5f, 0.5f}, {0.5f, height - 0.5f}, {width - 0.5f, height - 0.5f}, {width - 0.5f, 0.5f}
    };
    glStateCacheDisableLighting();
    glStateCacheDisableTex2D();
    glVertexPointer(2, GL_FLOAT, 0, square_vertices);
    glStateCacheEnableClientStateVertexArray();
    glColor4ub(255, 255, 255, 255);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
#endif
}