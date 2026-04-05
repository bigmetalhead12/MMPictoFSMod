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
#include "picto_export_png.h"

/***********************************************************************

	Export Pictobox Photo as PNG

***********************************************************************/

unsigned long str_byte_size(unsigned char* str) {
    unsigned long size = 0;
    while (str[size] != '\0') {
        size++;
    }
    return size;
}

/*======================
Export photo as PNG
======================*/

void export_photo(void* grayImgData, void* colorImgData) {
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
        REPY_Handle photo_handle = REPY_MemcpyToBytes(grayImgData, PICTO_PHOTO_SIZE, false);
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
        REPY_Handle photo_handle = REPY_MemcpyToBytes(colorImgData, SCREEN_WIDTH * SCREEN_HEIGHT * 2, false); // x2 as the image is 16 bit rather than 8 bit
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