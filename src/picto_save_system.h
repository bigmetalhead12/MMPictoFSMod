//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12, LT_Schmiddy
//
// 2026
//
// Load/Save and save-file handling for modified image.
//-----------------------------------------------------------------------------

// Headers
#include "modding.h"
#include "global.h"
#include "recompconfig.h"

#include "repy_api.h"       // REPY header file

/***********************************************************************

	Load/Save handling for modified images.

***********************************************************************/

bool loadsave_get_color_img(u32 saveslot, u16* data, size_t size);
void loadsave_set_color_img(u32 saveslot, u16* data, size_t size);