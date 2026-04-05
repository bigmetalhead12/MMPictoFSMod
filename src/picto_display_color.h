//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12 $
//
// 2026
//
// Display pictobox photo in color in the game
//-----------------------------------------------------------------------------

// Headers
#include "modding.h"
#include "global.h"
#include "recompconfig.h"
#include "sys_cfb.h"

#include "repy_api.h"       // REPY header file

/***********************************************************************

	Display pictobox photo with color in-game

***********************************************************************/

/*======================
Display during save prompt
======================*/

/**
 * @brief Save RGBA16 frame from prerender to buffer variable
 *
 * Converts prerender's RGBA16 frame into a u16-format
 *
 * @param pictoPhotoPrerender Prerender that has RGBA16 frame
 * @param colorBuffer Carries RGBA16 frame converted into u16
 * @param colorBufferFlag Bool flag that indicates if RGBA16
 *                        data is loaded into colorBuffer
 */
void preRender_to_buffer(PreRender* pictoPhotoPrerender, u16* colorBuffer, bool* colorBufferFlag);


/**
 * @brief Draw colored pictobox photo onto screen during save prompt
 *
 * This function posts the "Keep this picture?" prompt and loads
 * RGBA16 data from u16 buffer and draws it to screen.
 * 
 * Most of this code is copied from code segment of 'Interface_Draw'.
 *
 * @param play Current playstate
 * @param modPictoState Mod code's sPictoState variable, used to keep
 *                      track of sPictoState while sPictoState is marked
 *                      as 'off' to skip original function's photo draw
 * @param colorBuffer Contains u16-formatted RGBA16 data
 */
void draw_colored_photos(PlayState* play, s16* modPictoState, u16* colorBuffer);