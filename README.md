# Majora's Mask Recomp: Pictobox PNG Export Mod

This mod allows the user to save the photo from the pictobox into a local filesystem directory.

[LT_Schmiddy](https://github.com/LT-Schmiddy)'s [RecompExternalPython](https://thunderstore.io/c/zelda-64-recompiled/p/LT_Schmiddy/RecompExternalPython_for_Zelda64Recompiled/) (REPY) was used to make this mod.

# Installation
To install this mod, follow the instructions:

1. Download the NRM file
2. Open MM Recomp and select the Mods page
3. Click and drag the downloaded NRM file into the game.

# Required Download
To run this mod, download the [REPY](https://thunderstore.io/c/zelda-64-recompiled/p/LT_Schmiddy/RecompExternalPython_for_Zelda64Recompiled/) tool and install it into MM Recomp.

# Save Location
To locate the directory of the saved photos in your filesystem:
1. Open the mod menu in MM Recomp
2. In the mod menu, click 'Open Mods Folder'
3. Go to the parent directory
4. Go to the 'mod_data' directory
5. Go to the 'MM_PictoPhotos' directory

All photos are saved in this directory.

# Toggleable Features
This mod gives you control over how to save the pictobox's photo.

For in-game display options, you have two options:
* **Original** - Photos are shown as their original grayscale version in the game.
* **Color** - Photos are shown in color in the game.

For export options, you have two options:
* **Automatic** - Any photo taken with the pictobox, even if it is not saved in the game, is exported into the filesystem directory.
* **Selective** - Only photos that are saved in the game are exported into the filesystem directory.

For photo options, you have three options:
* **Original** - The photo is saved in grayscale with the pictobox resolution (160 x 112)
* **Color** - The photo is saved in color with the N64 resolution (320 x 240)
* **Both** - Both types of photos are saved

# Mod Template
This mod was created using [Wiseguy's MMRecompModTemplate](https://github.com/Zelda64Recomp/MMRecompModTemplate).

If you are new to MM Recomp Modding, it is crucial you understand how to work with this template before doing anything else. If you are able to build an NRM from this template on your machine, you are ready to mod the game.

# Acknowledgement
- To **LT_Schmiddy** for the Python REPY tool and implementing a persisting save for color photos in save file
- To **ThatHypedPerson** for implementing a method for saving colored photos
- To **Celeste** for providing coding help

# Feedback
If you notice any mistakes or omissions in the documentation, please leave a comment to report the issues. Thanks! 👍
