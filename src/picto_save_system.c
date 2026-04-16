//-----------------------------------------------------------------------------
//
// $ Author: Bigmetalhead12, LT_Schmiddy
//
// 2026
//
// Load/Save and save-file handling for modified image.
//-----------------------------------------------------------------------------

// Headers
#include "picto_save_system.h"
#include "picto_main.h"

#include "z64save.h"
#include "sys_flashrom.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"

/***********************************************************************

	Load/Save handling for modified images.

***********************************************************************/
static REPY_Handle sPictoLoadSaveController = REPY_NO_OBJECT;

REPY_ON_POST_INIT void init_loadsave_controller() {
    REPY_FN_SETUP;

    REPY_FN_EXEC_CACHE(init_loadsave_code,
        "import picto_save_file\n"
        "controller = picto_save_file.PictoLoadSaveController()"
    );

    // The handle created by REPY_FN_GET is not released during cleanup.
    sPictoLoadSaveController = REPY_FN_GET("controller");

    REPY_FN_CLEANUP;
}


// Savefile handling:
static void _update_savefile_location() {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    unsigned char* save_path = recomp_get_save_file_path();

    // Calling a member function of the object.
    REPY_CallAttrCStr(sPictoLoadSaveController, "update_picto_file_path",
        // Tuple with positional arguments:
        REPY_CreateTuple_SUH(1, REPY_CreateStr_SUH((const char*)save_path)), 
        // Dict with positional arguments (not used in this call):
        REPY_NO_OBJECT
    );
    recomp_free(save_path);

    REPY_PopInterpreter();
}


RECOMP_CALLBACK("*", recomp_on_load_save) void on_recomp_on_load_save() {
    REPY_FN_SETUP;
    _update_savefile_location();
    REPY_FN_SET("controller", sPictoLoadSaveController);
    REPY_FN_SET_U32("saveslot", gSaveContext.fileNum);

    // Copying data into mod memory, but only if the image exists.
    REPY_FN_IF_CACHE(has_slot_img, "controller.has_slot_img(saveslot)") {
        REPY_FN_EVAL_CACHE(load_slot_img, "controller.get_slot_img(saveslot)", slot_img);
        REPY_FN_DEFER_RELEASE(slot_img); // Make sure the `slot_img` handle is cleaned up with the scope.
        
        // `buffer` is any Python type that implements the Python buffer interface. `bytes` is one such type.
        REPY_MemcpyFromBuffer(inGameColorPhotoBuffer, sizeof(inGameColorPhotoBuffer), false, slot_img);
        viewColorPhotoAfterSave = true;
    } 

    REPY_FN_CLEANUP;
}


// Since we already have a controller object, we'll just call its methods directly since a full scope isn't necessary.
RECOMP_CALLBACK("*", recomp_on_owl_save) void on_recomp_on_owl_save() {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    _update_savefile_location();

    if (viewColorPhotoAfterSave) {
        REPY_Handle img_data = REPY_MemcpyToBytes(inGameColorPhotoBuffer, sizeof(inGameColorPhotoBuffer), false);
        REPY_CallAttrCStr(sPictoLoadSaveController, "set_slot_img", REPY_CreateTuple_SUH(2, REPY_CreateS32_SUH(gSaveContext.fileNum), img_data), REPY_NO_OBJECT);
        REPY_Release(img_data);
    }

    REPY_PopInterpreter();
}


#define SAVE_TYPE_AUTOSAVE 2
RECOMP_HOOK("Lib_MemCpy") void on_recomp_autosave() {
    // Cursed, but waiting for a better implementation.
    if (gSaveContext.save.isOwlSave != SAVE_TYPE_AUTOSAVE) {return;}

    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    _update_savefile_location();
    
    if (viewColorPhotoAfterSave) {
        REPY_Handle img_data = REPY_MemcpyToBytes(inGameColorPhotoBuffer, sizeof(inGameColorPhotoBuffer), false);
        REPY_CallAttrCStr(sPictoLoadSaveController, "set_slot_img", REPY_CreateTuple_SUH(2, REPY_CreateS32_SUH(gSaveContext.fileNum), img_data), REPY_NO_OBJECT);
        REPY_Release(img_data);
    }

    REPY_PopInterpreter();
}


RECOMP_HOOK_RETURN("Sram_SaveEndOfCycle") void on_Sram_SaveEndOfCycle() {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    _update_savefile_location();
    
    REPY_CallAttrCStr(sPictoLoadSaveController, "del_slot_img", REPY_CreateTuple_SUH(1, REPY_CreateS32_SUH(gSaveContext.fileNum)), REPY_NO_OBJECT);
    REPY_PopInterpreter();
}


// Need to add hooks for these as well.
RECOMP_HOOK("Sram_EraseSave") void pre_Sram_EraseSave(FileSelectState* fileSelect2, SramContext* sramCtx, s32 fileNum) {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    _update_savefile_location();
    
    REPY_CallAttrCStr(sPictoLoadSaveController, "del_slot_img", REPY_CreateTuple_SUH(1, REPY_CreateS32_SUH(fileNum)), REPY_NO_OBJECT);
    REPY_PopInterpreter();
}


RECOMP_HOOK("Sram_CopySave") void pre_Sram_CopySave(FileSelectState* fileSelect2, SramContext* sramCtx) {
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    _update_savefile_location();
    
    REPY_CallAttrCStr(sPictoLoadSaveController, "copy_slot_img", 
        REPY_CreateTuple_SUH(2, REPY_CreateS16_SUH(fileSelect2->selectedFileIndex), REPY_CreateS16_SUH(fileSelect2->copyDestFileIndex)), REPY_NO_OBJECT);
    REPY_PopInterpreter();
}