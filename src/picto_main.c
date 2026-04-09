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

// sPictoState is a global variable in the game code that indicates the pictobox state
extern s16 sPictoState;

// For displaying colored photos in-game
u16 inGameColorPhotoBuffer[PICTO_PHOTO_SIZE];   // Buffer that loads RGBA16 data from prerender frame
bool viewColorPhotoBeforeSave = false;          // Flag indicates if inGameColorPhotoBuffer img should be viewed before save
bool viewColorPhotoAfterSave = false;           // Flag indicates if inGameColorPhotoBuffer img should be viewed after save
s16 savedPictoState;                            // mod code's temp variable for sPictoState copy

// Game's buffer containing the pictobox photo's original I8 format
typedef union {
    u16 framebufferHiRes[HIRES_BUFFER_HEIGHT][HIRES_BUFFER_WIDTH] ALIGNED(64);
    struct {
        u8 pictoPhotoI8[PICTO_PHOTO_SIZE] ALIGNED(64);
        u8 D_80784600[0x56200] ALIGNED(64);
        u16 framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH] ALIGNED(64);
    };
} BufferHigh;

extern BufferHigh gHiBuffer;

// For saving colored photos as PNG
u16 colorPhotoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; // RGBA16 data from prerender frame with N64 resolution
PreRender* pictoPrerender;                          // Prerender frame which contains RGBA16 frame data


/*======================
Play_TakePictoPhoto
======================*/
// REPY
REPY_ADD_NRM_TO_MAIN_INTERPRETER;

// Store the PreRender to be used in a Play_TakePictoPhoto return hook
RECOMP_HOOK("Play_TakePictoPhoto") void on_Play_TakePictoPhoto(PreRender* prerender) {
    pictoPrerender = prerender;
    // note: the prerender can be saved here for an unsmoothed image
}


// Function that captures game frame into photo
RECOMP_HOOK_RETURN("Play_TakePictoPhoto") void return_Play_TakePictoPhoto() {   
    // For displaying colored photo, save the prerender data into a global variable
    if (recomp_get_config_u32("display_mode") == 1) {
        preRender_to_buffer(pictoPrerender, inGameColorPhotoBuffer, &viewColorPhotoBeforeSave);
    }
    
    // Run this code when "save mode" is set to "automatic" (0)
    if (recomp_get_config_u32("save_mode") == 0) {
        export_photo_to_png(gHiBuffer.pictoPhotoI8, pictoPrerender->fbufSave);
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
        // Right after photo was taken
        // Photo was taken but player does not keep the photo
        if (viewColorPhotoBeforeSave == 1 && sPictoState == PICTO_BOX_STATE_LENS) {
            viewColorPhotoBeforeSave = false;
            viewColorPhotoAfterSave = false;
        }
        // Photo was taken and player decides to save the photo
        if (viewColorPhotoBeforeSave == 1 && sPictoState == PICTO_BOX_STATE_OFF) {
            viewColorPhotoBeforeSave = false;
            viewColorPhotoAfterSave = true;
        }

        // Viewing saved photo
        // Player decides to not keep the photo
        if (viewColorPhotoAfterSave == 1 && sPictoState == PICTO_BOX_STATE_LENS) {
            viewColorPhotoBeforeSave = false;
            viewColorPhotoAfterSave = false;
        }
        // Player decides to keep the photo
        if (viewColorPhotoAfterSave == 1 && sPictoState == PICTO_BOX_STATE_OFF) {
            viewColorPhotoBeforeSave = false;
            viewColorPhotoAfterSave = true;
        }
    }
}


/*======================
Interface_Draw
======================*/

// Run before "Interface_Draw" draws pictobox photo's "Keep this picture?" prompt
RECOMP_HOOK("Interface_Draw") void on_Interface_Draw(PlayState* play) {
    // Exit this function if inGameColorPhotoBuffer is not filled or if photo is not taken yet
    if ((viewColorPhotoBeforeSave == 0 && viewColorPhotoAfterSave == 0) || sPictoState < PICTO_BOX_STATE_SETUP_PHOTO) {
        return;
    }
    // If inGameColorPhotoBuffer is filled, draw "Keep this picture?" and colored photo to game
    else {
        if (viewColorPhotoBeforeSave == 1 || viewColorPhotoAfterSave == 1) {
            draw_colored_photos(play, &savedPictoState, inGameColorPhotoBuffer);
        }
    }
}

// Revert sPictoState back to original value
RECOMP_HOOK_RETURN("Interface_Draw") void return_Interface_Draw(PlayState* play) {
    if (recomp_get_config_u32("display_mode") == 1) {
        // Exit this function if inGameColorPhotoBuffer is not filled
        if (viewColorPhotoBeforeSave == 0 && viewColorPhotoAfterSave == 0) {
            return;
        }

        // If inGameColorPhotoBuffer is filled, update sPictoState again
        if (savedPictoState >= PICTO_BOX_STATE_SETUP_PHOTO) {
            sPictoState = savedPictoState;
            savedPictoState = PICTO_BOX_STATE_OFF;      // Had to manually set this to off to make the draw function to work at the proper time
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
        export_photo_to_png(srcI8, colorPhotoBuffer);
    }
}