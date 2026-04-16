"""

Author: Bigmetalhead12, LT_Schmiddy

2026

Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
"""

import os, shutil
from pathlib import Path

# Use this as the file extension for saved images.
PICTO_SAVE_SUFFIX = ".picto"

# Handles all save-data stuff from the picto mod. Methods are invoked from mod code.
class PictoLoadSaveController:
    _save_path: Path
    
    # Defining __slots__ improves performance of class member access slightly.
    __slots__ = (
        "_save_path"
    )
    
    def __init__(self, file_path: Path = None):
        self._save_path = file_path
        
    def _get_saveslot_key(self, saveslot_num: int) -> str:
        return f"saveslot-{saveslot_num}"
    
    def _get_picto_save_dir(self) -> Path:
        return self._save_path.parent
        
    def _get_picto_save_path(self, saveslot_key: int) -> Path:
        save_dir = self._save_path.parent
        # Save file name format is {saveslot key}.{save file name without .bin}.picto
        save_name = f"{saveslot_key}.{self._save_path.name}"
        return save_dir.joinpath(save_name).with_suffix(PICTO_SAVE_SUFFIX)
    
    # Making sure this always corresponds to the correct save file.
    def update_picto_file_path(self, save_path: str):
        self._save_path = Path(save_path)
    
    # Updating the images on disk:
    def has_slot_img(self, slot: int) -> bool:
        key = self._get_saveslot_key(slot)
        return self._get_picto_save_path(key).exists()
    
    def get_slot_img(self, slot: int) -> bytes:
        key = self._get_saveslot_key(slot)
        return self._get_picto_save_path(key).read_bytes()
    
    def set_slot_img(self, slot: int, data: bytes):
        key = self._get_saveslot_key(slot)
        self._get_picto_save_path(key).write_bytes(data)
    
    def del_slot_img(self, slot: int):
        key = self._get_saveslot_key(slot)
        fpath = self._get_picto_save_path(key)
        if fpath.exists():
            os.remove(fpath)
    
    def copy_slot_img(self, src_slot: int, dst_slot: int):
        src_key = self._get_saveslot_key(src_slot)
        src_fpath = self._get_picto_save_path(src_key)
        if src_fpath.exists():
            dst_key = self._get_saveslot_key(dst_slot)
            dst_fpath = self._get_picto_save_path(dst_key)
            shutil.copy(src_fpath, dst_fpath)
