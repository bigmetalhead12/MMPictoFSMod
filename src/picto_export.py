"""

Author: Bigmetalhead12

2026

Pictobox PNG Export Mod (using LT_Schmiddy's REPY)
API Source: https://github.com/LT-Schmiddy/zelda64recomp-python-extlibs-mod
"""

# Imported Packages
import picto_png as png         # Python file saved in this directory
import itertools
from pathlib import Path
from datetime import datetime

# Macros
PICTO_PHOTO_WIDTH   = 160
PICTO_PHOTO_HEIGHT  = 112

def pictobox_to_directory(pictobox_photo, save_directory):
    # Organize pictobox photo data into rows based on PICTO_PHOTO_WIDTH
    rows = itertools.batched(pictobox_photo, PICTO_PHOTO_WIDTH)

    # Set PNG writer
    writer = png.Writer(
        width=PICTO_PHOTO_WIDTH,
        height=PICTO_PHOTO_HEIGHT,
        bitdepth=8,
    )

    # Prepare directory to save photo as PNG file
    base_dir = Path(save_directory.decode("utf-8")).parent
    photo_dir = base_dir / "MM_PictoPhotos"
    photo_dir.mkdir(exist_ok=True)

    # Prepare PNG file name
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    output_path = photo_dir / f"picto_photo_{timestamp}.png"

    # Write photo data into PNG file
    with open(output_path, "wb") as f:
        writer.write(f, rows)