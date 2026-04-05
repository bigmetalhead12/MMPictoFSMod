//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12 $
//
// 2026
//
// Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
// API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
//-----------------------------------------------------------------------------

// Headers
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "sys_cfb.h"

#include "repy_api.h"       // REPY header file

#include "picto_export_png.h"
#include "picto_display_color.h"

/***********************************************************************

	Pictobox Photo Mod

***********************************************************************/

// Pictobox Photo Display
typedef enum {
    /* 0 */ PICTO_BOX_STATE_OFF,         // Not using the pictograph
    /* 1 */ PICTO_BOX_STATE_LENS,        // Looking through the lens of the pictograph
    /* 2 */ PICTO_BOX_STATE_SETUP_PHOTO, // Looking at the photo currently taken
    /* 3 */ PICTO_BOX_STATE_PHOTO
} PictoBoxState;

extern s16 sPictoState;
extern s16 sPictoPhotoBeingTaken;

// For displaying colored photos in-game
u16 inGameColorPhotoBuffer[PICTO_PHOTO_SIZE];
bool inGameColorPhotoReady = false;     // flag that indicates if inGameColorPhotoBuffer is filled
s16 savedPictoState;    // mod code's temp variable for sPictoState copy


// From 'buffers.h'
typedef union {
    u16 framebufferHiRes[HIRES_BUFFER_HEIGHT][HIRES_BUFFER_WIDTH] ALIGNED(64);
    struct {
        u8 pictoPhotoI8[PICTO_PHOTO_SIZE] ALIGNED(64);
        u8 D_80784600[0x56200] ALIGNED(64);
        u16 framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH] ALIGNED(64);
    };
} BufferHigh;

extern BufferHigh gHiBuffer;


// REPY
REPY_ADD_NRM_TO_MAIN_INTERPRETER;

// For saving colored photos
u16 colorPhotoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
PreRender* pictoPrerender;


/*======================
Play_TakePictoPhoto
======================*/

// Store the PreRender to be used in a Play_TakePictoPhoto return hook
RECOMP_HOOK("Play_TakePictoPhoto") void on_Play_TakePictoPhoto(PreRender* prerender) {
    pictoPrerender = prerender;
    // note: the prerender can be saved here for an unsmoothed image
}


// Function that captures game frame into photo
RECOMP_HOOK_RETURN("Play_TakePictoPhoto") void return_Play_TakePictoPhoto() {   
    // For displaying colored photo, save the prerender data into a global variable
    if (recomp_get_config_u32("display_mode") == 1) {
        preRender_to_buffer(pictoPrerender, inGameColorPhotoBuffer, &inGameColorPhotoReady);
    }
    
    // Run this code when "save mode" is set to "automatic" (0)
    if (recomp_get_config_u32("save_mode") == 0) {
        export_photo(gHiBuffer.pictoPhotoI8, pictoPrerender->fbufSave);
    }

    // Run this code when "save mode" is set to "selective" (1)
    else if (recomp_get_config_u32("save_mode") == 1) {
        // Save the prerender data into a global variable
        for (s32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            colorPhotoBuffer[i] = pictoPrerender->fbufSave[i];
        }
    }
}


/*======================
Interface_UpdateButtonsPart1
======================*/

RECOMP_HOOK_RETURN("Interface_UpdateButtonsPart1") void return_Interface_UpdateButtonsPart1(PlayState* play) {
    // Exit the "Keep this picture?" prompt properly after taking a photo 
    if (recomp_get_config_u32("display_mode") == 1) {
        // Photo was taken but player does not keep the photo
        if (inGameColorPhotoReady == 1 && sPictoState == PICTO_BOX_STATE_LENS) {
            inGameColorPhotoReady = false;
        }
        // Photo was taken and player decides to save the photo
        if (inGameColorPhotoReady == 1 && sPictoState == PICTO_BOX_STATE_OFF) {
            inGameColorPhotoReady = false;
        }
    }
}


/*======================
Interface_Draw
======================*/

// Run before "Interface_Draw" draws pictobox photo's "Keep this picture?" prompt
RECOMP_HOOK("Interface_Draw") void on_Interface_Draw(PlayState* play) {
    // Exit this function if inGameColorPhotoBuffer is not filled or if photo is not taken yet
    if (!inGameColorPhotoReady || sPictoState < PICTO_BOX_STATE_SETUP_PHOTO) {
        return;
    }
    // If inGameColorPhotoBuffer is filled, draw "Keep this picture?" and colored photo to game
    else {
        draw_colored_photos(play, &savedPictoState, inGameColorPhotoBuffer);
    }
}

// Revert sPictoState back to original value
RECOMP_HOOK_RETURN("Interface_Draw") void return_Interface_Draw(PlayState* play) {
    if (recomp_get_config_u32("display_mode") == 1) {
        // Exit this function if inGameColorPhotoBuffer is not filled
        if (inGameColorPhotoReady == 0) {
            return;
        }

        // Restore the sPictoState value after Interface_Draw function is run
        if (savedPictoState >= PICTO_BOX_STATE_SETUP_PHOTO) {
            sPictoState = savedPictoState;
        }
    }
}


/*======================
Play_CompressI8ToI5
======================*/
// Compresses I8 photo image to I5 after deciding to save pictobox photo in game
RECOMP_HOOK("Play_CompressI8ToI5") void on_Play_CompressI8ToI5(void* srcI8, void* destI5, size_t size) {
    // Run this code when "save mode" is set to "selective" (1)
    if (recomp_get_config_u32("save_mode")) {
        // Export pictobox photo as PNG before I8 photo gets compressed
        export_photo(srcI8, colorPhotoBuffer);
    }
}