//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12 $
//
// 2026
//
// Display pictobox photo in color in the game
//-----------------------------------------------------------------------------

// Headers
#include "picto_display_color.h"

/***********************************************************************

	Display pictobox photo with color in-game

***********************************************************************/

typedef enum {
    /* 0 */ PICTO_BOX_STATE_OFF,         // Not using the pictograph
    /* 1 */ PICTO_BOX_STATE_LENS,        // Looking through the lens of the pictograph
    /* 2 */ PICTO_BOX_STATE_SETUP_PHOTO, // Looking at the photo currently taken
    /* 3 */ PICTO_BOX_STATE_PHOTO
} PictoBoxState;

// sPictoState is a global variable in the game code that indicates the pictobox state
extern s16 sPictoState;


/*======================
Display during save prompt
======================*/
void preRender_to_buffer(PreRender* pictoPhotoPrerender, u16* colorBuffer, bool* colorBufferFlag) {
    s32 bufferIndex = 0;
    // Iterate through photo's rows
    for (s32 i = PICTO_PHOTO_TOPLEFT_Y; i < (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT); i++) {
        // Iterate through pixels in each row
        for (s32 j = PICTO_PHOTO_TOPLEFT_X; j < (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH); j++) {
            colorBuffer[bufferIndex++] = pictoPhotoPrerender->fbufSave[i * SCREEN_WIDTH + j];
        }
    }
    *colorBufferFlag = true;   // flag to indicate that colored photo data is loaded into buffer
}


void draw_colored_photos(PlayState* play, s16* modPictoState, u16* colorBuffer) {
    Player* player = GET_PLAYER(play);
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    // Some form of check
    if (!(play->actorCtx.flags & ACTORCTX_FLAG_PICTO_BOX_ON)) {
        Play_CompressI8ToI5((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : gWorkBuffer,
                            (u8*)gSaveContext.pictoPhotoI5, PICTO_PHOTO_WIDTH * PICTO_PHOTO_HEIGHT);

        interfaceCtx->bButtonInterfaceDoActionActive = interfaceCtx->bButtonInterfaceDoAction = 0;

        sPictoState = PICTO_BOX_STATE_OFF;
        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
    // Actual drawing code segment
    else {
        s16 pictoRectTop;
        s16 pictoRectLeft;

        // Save the real sPictoState value into mod code flag
        *modPictoState = sPictoState;

        // Put up "Keep this picture?" prompt
        if (sPictoState == PICTO_BOX_STATE_SETUP_PHOTO) {
            sPictoState = PICTO_BOX_STATE_PHOTO;
            Message_StartTextbox(play, 0xF8, NULL);
            Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
            player->stateFlags1 |= PLAYER_STATE1_200;

            // Update mod code's sPictoState copy
            *modPictoState = PICTO_BOX_STATE_PHOTO;
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
                                (u16*)(colorBuffer) + (0x500 * sp2CC),
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