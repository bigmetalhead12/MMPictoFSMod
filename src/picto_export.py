"""

Author: Bigmetalhead12, ThatHypedPerson

2026

Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
"""

# Imported Packages
import picto_png as png         # Python file saved in this directory
import itertools
from pathlib import Path
from datetime import datetime


# Macros for pictobox photo
PICTO_PHOTO_WIDTH   = 160
PICTO_PHOTO_HEIGHT  = 112

def pictobox_to_directory(pictobox_photo, save_directory):
    """Export a grayscale pictobox photo as a PNG into the indicated filesystem directory

    Args:
        pictobox_photo (py_object): The I8 pictobox photo
        save_directory (py_object): Filesystem directory location
    """
    # Organize pictobox photo data into rows based on PICTO_PHOTO_WIDTH
    rows = itertools.batched(pictobox_photo, PICTO_PHOTO_WIDTH)

    # Set PNG writer
    writer = png.Writer(
        width=PICTO_PHOTO_WIDTH,
        height=PICTO_PHOTO_HEIGHT,
        bitdepth=8,
    )

    # Prepare directory to save photo as PNG file
    photo_dir = Path(save_directory.decode("utf-8")).parent.joinpath("mod_data", "MM_PictoPhotos").resolve()
    photo_dir.mkdir(parents=True, exist_ok=True)

    # Prepare PNG file name
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    output_path = photo_dir / f"picto_photo_{timestamp}.png"

    # Write photo data into PNG file
    with open(output_path, "wb") as f:
        writer.write(f, rows)


# Macros for N64 screen resolution
N64_SCREEN_WIDTH    = 320
N64_SCREEN_HEIGHT   = 240

def prerender_to_directory(pictobox_photo, save_directory):
    """Export an RGB16 prerender as a PNG into the indicated filesystem directory

    Args:
        pictobox_photo (py_object): The RGB prerender data from taking pictobox photo
        save_directory (py_object): Filesystem directory location
    """
    # Organize pictobox photo data into rows based on N64_SCREEN_WIDTH
    rows = itertools.batched(pictobox_photo, N64_SCREEN_WIDTH * 2) # hack fix to account for 16 bit data rather than 8 bit
    rgb_rows = []

    # Convert each row from RGBA16 to RGB
    for row in rows:
        new_row = []
        for i in range(0, len(row), 2): # hack fix to account for 16 bit data rather than 8 bit
            pixel = row[i] << 8 | row[i+1]
            red = ((((pixel) >> 11) & 0x1F) * 255 + 15) // 31
            green = ((((pixel) >> 6) & 0x1F) * 255 + 15) // 31
            blue = ((((pixel) >> 1) & 0x1F) * 255 + 15) // 31
            new_row.extend([red, green, blue])
        rgb_rows.append(new_row)

    # Set PNG writer
    writer = png.Writer(
        width=N64_SCREEN_WIDTH,
        height=N64_SCREEN_HEIGHT,
        bitdepth=8,
        greyscale=False,
    )

    # Prepare directory to save photo as PNG file
    photo_dir = Path(save_directory.decode("utf-8")).parent.joinpath("mod_data", "MM_PictoPhotos").resolve()
    photo_dir.mkdir(parents=True, exist_ok=True)

    # Prepare PNG file name
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    output_path = photo_dir / f"picto_photo_rgb_{timestamp}.png"

    # Write photo data into PNG file
    with open(output_path, "wb") as f:
        writer.write(f, rgb_rows)