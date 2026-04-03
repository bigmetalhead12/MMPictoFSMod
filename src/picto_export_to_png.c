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

#include "repy_api.h"       // REPY header file

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

PreRender* pictoPrerender;

// Store the PreRender to be used in a Play_TakePictoPhoto return hook
RECOMP_HOOK("Play_TakePictoPhoto") void on_Play_TakePictoPhoto(PreRender* prerender) {
    pictoPrerender = prerender;
    // note: the prerender can be saved here for an unsmoothed image
}


// Automatic - Export any photo as PNG to local directory regardless of it being saved or not in-game
RECOMP_HOOK_RETURN("Play_TakePictoPhoto") void return_Play_TakePictoPhoto() {    
    // Run this code when "save mode" is set to "automatic" (0)
    if (!recomp_get_config_u32("save_mode")) {
        // Get the path to the mod folder
        unsigned char* mod_folder_path = recomp_get_mod_folder_path();
        unsigned long size = str_byte_size(mod_folder_path);

        REPY_FN_SETUP;
        // Copy mod folder path string into Python object
        REPY_Handle folder_path_handle = REPY_MemcpyToBytes(mod_folder_path, size, false);
        REPY_FN_SET("mod_folder_path", folder_path_handle);

        if (recomp_get_config_u32("auto_save_type") == 0 || recomp_get_config_u32("auto_save_type") == 2) { // Original / Both
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

        if (recomp_get_config_u32("auto_save_type") == 1 || recomp_get_config_u32("auto_save_type") == 2) { // Color / Both
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

        REPY_FN_CLEANUP;
    }
}