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

/***********************************************************************

	Get current pictobox state

***********************************************************************/

typedef enum {
    /* 0 */ PICTO_BOX_STATE_OFF,         // Not using the pictograph
    /* 1 */ PICTO_BOX_STATE_LENS,        // Looking through the lens of the pictograph
    /* 2 */ PICTO_BOX_STATE_SETUP_PHOTO, // Looking at the photo currently taken
    /* 3 */ PICTO_BOX_STATE_PHOTO
} PictoBoxState;

extern s16 sPictoState;
extern s16 sPictoPhotoBeingTaken;
s16 savedPictoState;    // mod code's temp variable for sPictoState copy


/***********************************************************************

	Export Pictobox Photo as PNG

***********************************************************************/

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

// Function that finds byte size of given string
unsigned long str_byte_size(unsigned char* str) {
    unsigned long size = 0;
    while (str[size] != '\0') {
        size++;
    }
    return size;
}


// Save Photo based on toggled option in mod's config
REPY_ADD_NRM_TO_MAIN_INTERPRETER;

// For saving colored photos
u16 colorPhotoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
PreRender* pictoPrerender;

// For displaying colored photos
u16 inGameColorPhotoBuffer[PICTO_PHOTO_SIZE];
bool inGameColorPhotoReady = false;


// Store the PreRender to be used in a Play_TakePictoPhoto return hook
RECOMP_HOOK("Play_TakePictoPhoto") void on_Play_TakePictoPhoto(PreRender* prerender) {
    pictoPrerender = prerender;
    // note: the prerender can be saved here for an unsmoothed image
}


// Automatic - Export any photo as PNG to local directory regardless of it being saved or not in-game
RECOMP_HOOK_RETURN("Play_TakePictoPhoto") void return_Play_TakePictoPhoto() {   
    // For displaying colored photo, save the prerender data into a global variable
    s32 bufferIndex = 0;
    // Iterate through photo's rows
    for (s32 i = PICTO_PHOTO_TOPLEFT_Y; i < (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT); i++) {
        // Iterate through pixels in each row
        for (s32 j = PICTO_PHOTO_TOPLEFT_X; j < (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH); j++) {
            inGameColorPhotoBuffer[bufferIndex++] = pictoPrerender->fbufSave[i * SCREEN_WIDTH + j];
        }
    }
    inGameColorPhotoReady = true;   // flag to indicate that colored photo data is loaded into buffer
    
    // Run this code when "save mode" is set to "automatic" (0)
    if (!recomp_get_config_u32("save_mode")) {
        // Get the path to the mod folder
        unsigned char* mod_folder_path = recomp_get_mod_folder_path();
        unsigned long size = str_byte_size(mod_folder_path);

        REPY_FN_SETUP;
        // Copy mod folder path string into Python object
        REPY_Handle folder_path_handle = REPY_MemcpyToBytes(mod_folder_path, size, false);
        REPY_FN_SET("mod_folder_path", folder_path_handle);

        // If the Color Option is set to "Original" or "Both"
        if (recomp_get_config_u32("save_type") == 0 || recomp_get_config_u32("save_type") == 2) {
            // Copy pictobox photo data into Python object
            REPY_Handle photo_handle = REPY_MemcpyToBytes(gHiBuffer.pictoPhotoI8, PICTO_PHOTO_SIZE, false);
            REPY_FN_SET("pictobox_photo", photo_handle);

            //Python code here
            REPY_FN_EXEC_CACHE(pictobox_save,
                "import picto_export\n"
                "picto_export.pictobox_to_directory(pictobox_photo, mod_folder_path)\n"
            );

            // Release pictobox photo Python object
            REPY_Release(photo_handle);
        }

        // If the Color Option is set to "Color" or "Both"
        if (recomp_get_config_u32("save_type") == 1 || recomp_get_config_u32("save_type") == 2) {
            // Copy prerender data into Python object
            REPY_Handle photo_handle = REPY_MemcpyToBytes(pictoPrerender->fbufSave, SCREEN_WIDTH * SCREEN_HEIGHT * 2, false); // x2 as the image is 16 bit rather than 8 bit
            REPY_FN_SET("pictobox_photo", photo_handle);

            //Python code here
            REPY_FN_EXEC_CACHE(pictobox_save,
                "import picto_export\n"
                "picto_export.prerender_to_directory(pictobox_photo, mod_folder_path)\n"
            );

            // Release pictobox photo Python object
            REPY_Release(photo_handle);
        }

        REPY_FN_CLEANUP;
    }

    // Run this code when "save mode" is set to "selective" (1)
    else {
        // Save the prerender data into a global variable
        for (s32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            colorPhotoBuffer[i] = pictoPrerender->fbufSave[i];
        }
    }
}


// Exit the "Keep this picture?" prompt properly after taking a photo 
RECOMP_HOOK_RETURN("Interface_UpdateButtonsPart1") void return_Interface_UpdateButtonsPart1(PlayState* play) {
    // Photo was taken but player does not keep the photo
    if (inGameColorPhotoReady == 1 && sPictoState == PICTO_BOX_STATE_LENS) {
        inGameColorPhotoReady = false;
    }
    // Photo was taken and player decides to save the photo
    if (inGameColorPhotoReady == 1 && sPictoState == PICTO_BOX_STATE_OFF) {
        inGameColorPhotoReady = false;
    }
}


// Draw the colored picture and the photo save prompt before Interface_Draw and
// prevent the original function from drawing the I8 pictobox photo and save prompt
RECOMP_HOOK("Interface_Draw") void on_Interface_Draw(PlayState* play) {
    // Exit this function if inGameColorPhotoBuffer is not filled or if photo is not taken yet
    if (!inGameColorPhotoReady || sPictoState < PICTO_BOX_STATE_SETUP_PHOTO) {
        return;
    }

    Player* player = GET_PLAYER(play);
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    // Code block from Interface_Draw that draws the pictobox photo in the photo save prompt
    // Nearly all of this is the copy and pasted from decomp except for gDPLoadTextureBlock
    if (!(play->actorCtx.flags & ACTORCTX_FLAG_PICTO_BOX_ON)) {
        Play_CompressI8ToI5((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : gWorkBuffer,
                            (u8*)gSaveContext.pictoPhotoI5, PICTO_PHOTO_WIDTH * PICTO_PHOTO_HEIGHT);

        interfaceCtx->bButtonInterfaceDoActionActive = interfaceCtx->bButtonInterfaceDoAction = 0;

        sPictoState = PICTO_BOX_STATE_OFF;
        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
    else {
        s16 pictoRectTop;
        s16 pictoRectLeft;

        // Save the real sPictoState value into temporary mod code variable
        savedPictoState = sPictoState;

        // Put up "Keep this picture?" prompt
        if (sPictoState == PICTO_BOX_STATE_SETUP_PHOTO) {
            sPictoState = PICTO_BOX_STATE_PHOTO;
            Message_StartTextbox(play, 0xF8, NULL);
            Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
            player->stateFlags1 |= PLAYER_STATE1_200;

            // Update mod code's sPictoState copy
            savedPictoState = PICTO_BOX_STATE_PHOTO;
        }

        // Draw the colored photo
        OPEN_DISPS(play->state.gfxCtx);

        gDPPipeSync(OVERLAY_DISP++);
        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 200, 200, 250);
        gDPFillRectangle(OVERLAY_DISP++, 70, 22, 251, 151);

        Gfx_SetupDL39_Overlay(play->state.gfxCtx);

        gDPSetRenderMode(OVERLAY_DISP++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);

        s16 sp2CC;
        s16 pictoRectTop = PICTO_PHOTO_TOPLEFT_Y - 33;
        s16 pictoRectLeft;

        for (sp2CC = 0; sp2CC < (PICTO_PHOTO_HEIGHT / 8); sp2CC++, pictoRectTop += 8) {
            pictoRectLeft = PICTO_PHOTO_TOPLEFT_X;

            // Changed G_IM_FMT_I to G_IM_FMT_RGBA and G_IM_SIZ_8b to G_IM_SIZ_16b
            gDPLoadTextureBlock(OVERLAY_DISP++,
                                (u16*)(inGameColorPhotoBuffer) + (0x500 * sp2CC),
                                G_IM_FMT_RGBA, G_IM_SIZ_16b, PICTO_PHOTO_WIDTH, 8, 0, G_TX_NOMIRROR | G_TX_WRAP, 
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSPTextureRectangle(OVERLAY_DISP++, pictoRectLeft << 2, pictoRectTop << 2,
                                (pictoRectLeft + PICTO_PHOTO_WIDTH) << 2, (pictoRectTop << 2) + (8 << 2),
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        }

        CLOSE_DISPS(play->state.gfxCtx);
    }

    // Turn sPictoState to PICTO_BOX_STATE_OFF to skip pictobox photo drawing in original Interface_Draw
    sPictoState = PICTO_BOX_STATE_OFF;
}

// Revert sPictoState back to original value
RECOMP_HOOK_RETURN("Interface_Draw") void return_Interface_Draw(PlayState* play) {
    // Exit this function if inGameColorPhotoBuffer is not filled
    if (inGameColorPhotoReady == 0) {
        return;
    }

    // Restore the sPictoState value after Interface_Draw function is run
    if (savedPictoState >= PICTO_BOX_STATE_SETUP_PHOTO) {
        sPictoState = savedPictoState;
    }
}


// Selective - Export any photo as PNG to local directory regardless of it being saved or not in-game
RECOMP_HOOK("Play_CompressI8ToI5") void on_Play_CompressI8ToI5(void* srcI8, void* destI5, size_t size) {
    // Run this code when "save mode" is set to "selective" (1)
    if (recomp_get_config_u32("save_mode")) {
        // Get the path to the mod folder
        unsigned char* mod_folder_path = recomp_get_mod_folder_path();
        unsigned long size_of_string = str_byte_size(mod_folder_path);

        REPY_FN_SETUP;
        // Copy mod folder path string into Python object
        REPY_Handle folder_path_handle = REPY_MemcpyToBytes(mod_folder_path, size_of_string, false);
        REPY_FN_SET("mod_folder_path", folder_path_handle);

        // If the Color Option is set to "Original" or "Both"
        if (recomp_get_config_u32("save_type") == 0 || recomp_get_config_u32("save_type") == 2) {
            // Copy pictobox photo data into Python object
            REPY_Handle photo_handle = REPY_MemcpyToBytes(srcI8, PICTO_PHOTO_SIZE, false);
            REPY_FN_SET("pictobox_photo", photo_handle);

            //Python code here
            REPY_FN_EXEC_CACHE(pictobox_save,
                "import picto_export\n"
                "picto_export.pictobox_to_directory(pictobox_photo, mod_folder_path)\n"
            );

            // Release pictobox photo Python object
            REPY_Release(photo_handle);
        }

        // If the Color Option is set to "Color" or "Both"
        if (recomp_get_config_u32("save_type") == 1 || recomp_get_config_u32("save_type") == 2) { 
            // Copy prerender data into Python object
            REPY_Handle photo_handle = REPY_MemcpyToBytes(colorPhotoBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2, false); // x2 as the image is 16 bit rather than 8 bit
            REPY_FN_SET("pictobox_photo", photo_handle);

            //Python code here
            REPY_FN_EXEC_CACHE(pictobox_save,
                "import picto_export\n"
                "picto_export.prerender_to_directory(pictobox_photo, mod_folder_path)\n"
            );

            // Release pictobox photo Python object
            REPY_Release(photo_handle);
        }

        REPY_FN_CLEANUP;
    }
}