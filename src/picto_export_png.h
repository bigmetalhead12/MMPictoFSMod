//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12, ThatHypedPerson $
//
// 2026
//
// Export pictobox photo as grayscaled or colored PNG in filesystem
// directory
//-----------------------------------------------------------------------------

// Headers
#include "modding.h"
#include "global.h"
#include "recompconfig.h"

#include "repy_api.h"       // REPY header file

/***********************************************************************

	Export Pictobox Photo as PNG

***********************************************************************/

/**
 * @brief Measures size of given string value
 *
 * @param str Input string that is to be measured
 * @return byte length of str
 */
unsigned long str_byte_size(unsigned char* str);


/*======================
Export photo as PNG
======================*/

/**
 * @brief Export pictobox photo as PNG
 *
 * Export pictobox photo as grayscaled or colored PNG based on
 * configuration values
 *
 * @param grayImgData I8-formatted pictobox photo
 * @param colorImgData u16-formatted RGBA16 prerender buffer img
 */
void export_photo(void* grayImgData, void* colorImgData);
