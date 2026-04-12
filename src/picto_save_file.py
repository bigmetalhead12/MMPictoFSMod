"""

Author: Bigmetalhead12, LT_Schmiddy

2026

Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
"""

import os
from pathlib import Path

# Use this as the file extension for saved images.
PICTO_SAVE_SUFFIX = ".picto"

# Handles all save-data stuff from the picto mod. Methods are invoked from mod code.
class PictoLoadSaveController:
    _save_path: Path
    _loaded_images: dict[str, bytes]
    
    # Defining __slots__ improves performance of class member access slightly.
    __slots__ = (
        "_save_path",
        "_loaded_images"
    )
    
    def __init__(self, file_path: Path = None):
        self._save_path = file_path
        self._loaded_images = {}
        
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
    
    # Updating the images in memory:
    def has_slot_img(self, slot: int) -> bool:
        key = self._get_saveslot_key(slot)
        return key in self._loaded_images
    
    def get_slot_img(self, slot: int) -> bytes:
        key = self._get_saveslot_key(slot)
        return self._loaded_images[key] 
    
    def set_slot_img(self, slot: int, data: bytes):
        # print(data)
        self._loaded_images[self._get_saveslot_key(slot)] = data
    
    def del_slot_img(self, slot: int):
        del self._loaded_images[self._get_saveslot_key(slot)]
        
    def clear_all_imgs(self):
        self._loaded_images.clear()
    
    # Actual file access:
    def on_game_load(self, slot: int):
        save_dir = self._get_picto_save_dir()
        for picto_file in [save_dir.joinpath(i) for i in os.scandir(save_dir)]:
            if picto_file.suffix == PICTO_SAVE_SUFFIX:
                slot_key = picto_file.name.split(".")[0]
                self._loaded_images[slot_key] = picto_file.read_bytes()
    
    def on_game_owlsave(self, slot: int):            
        for slot_key, data in self._loaded_images.items():
            self._get_picto_save_path(slot_key).write_bytes(data)
        
    def on_game_sotsave(self, slot: int):            
        save_dir = self._get_picto_save_dir()
        for picto_file in [save_dir.joinpath(i) for i in os.scandir(save_dir)]:
            if picto_file.suffix == PICTO_SAVE_SUFFIX:
                os.remove(picto_file)
        
        
    