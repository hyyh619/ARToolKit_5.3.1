#ifndef __NFT_SIMPLE_H
#define __NFT_SIMPLE_H

// ============================================================================
// Includes
// ============================================================================
#include <AR/ar.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Constants
// ============================================================================
#define PAGES_MAX 10                        // Maximum number of pages expected. You can change this down (to save memory) or up (to accomodate more pages.)


// ============================================================================
// Function prototypes
// ============================================================================
int InitNFT(ARParamLT *cparamLT, AR_PIXEL_FORMAT pixFormat);
int LoadNFTData(void);
int UnloadNFTData(void);


// ============================================================================
// global variables declaration
// ============================================================================
extern ARMarkerNFT      *g_pMarkersNFT;
extern int              g_nMarkersNFTCount;
extern THREAD_HANDLE_T  *g_threadHandle;
extern AR2HandleT       *g_ar2Handle;
extern KpmHandle        *g_kpmHandle;
extern int              g_surfaceSetCount;
extern AR2SurfaceSetT   *g_surfaceSet[PAGES_MAX];

#ifdef __cplusplus
}
#endif

#endif /* __NFT_SIMPLE_H */