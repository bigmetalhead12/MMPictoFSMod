#ifndef __PICTO_MAIN__
#define __PICTO_MAIN__

//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12 $
//
// 2026
//
// Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
// API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
//-----------------------------------------------------------------------------

#include "global.h"


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
extern u16 inGameColorPhotoBuffer[PICTO_PHOTO_SIZE];   // Buffer that loads RGBA16 data from prerender frame
extern bool viewColorPhotoBeforeSave;          // Flag indicates if inGameColorPhotoBuffer img should be viewed before save
extern bool viewColorPhotoAfterSave;           // Flag indicates if inGameColorPhotoBuffer img should be viewed after save

#endif