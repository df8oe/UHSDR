# Tool to extract 8x8 font data, save to bitmap file, and apply modifications
# to source code after editing the bitmap.

from __future__ import print_function
from matplotlib.pyplot import imread, imsave, imshow, show
import numpy as np
import sys

# Where to find the font data - may need updated if code has changed.
source_file = 'ui_lcd_hy28_fonts.c'
start_marker = 'const uint8_t GL_ASCII8x8_Table [] ='
start_offset = 2  # Data starts this number of lines after start marker.
end_marker = '};' # Indicates end of font data.

# Image filename used in extract and insert modes.
image_file = 'font-8x8.png'

mode = None
if len(sys.argv) > 1:
    mode = sys.argv[1]

if mode not in ('show', 'extract', 'insert'):
    print("Usage: %s { show | extract | insert }" % sys.argv[0])
    sys.exit(1)

# Get the literals used to populate the font array in the source file
lines = [line.rstrip() for line in open(source_file).readlines()]
start = lines.index(start_marker) + start_offset
end = start + lines[start:].index(end_marker)
data = str.join("", lines[start:end])

# Eval the literals to get the values into a numpy array
packed = eval("np.array([%s], np.uint8)" % data)

# Reorganise into a monochrome image, with the 96 8 x 8 characters
# laid out in 8 rows by 12 columns for easier viewing/editing
unpacked = np.unpackbits(packed)
bitmaps = unpacked.reshape(96, 8, 8)
indices = np.arange(96).reshape(8, 12)
image = np.block([[bitmaps[idx] for idx in row] for row in indices])

if mode == 'show':
    # Display font image
    imshow(image, cmap='binary')
    show()

elif mode == 'extract':
    # Save font image
    imsave(image_file, image, format='png', cmap='binary')

elif mode == 'insert':
    # Read in modified font image
    image = imread(image_file)[:,:,0].astype(bool)

    # Reorganise back to original order
    bitmaps = np.vstack([np.vstack(np.split(row, 12, 1))
                         for row in np.split(image, 8)])
    unpacked = bitmaps.reshape(-1)
    packed = ~np.packbits(unpacked)

    # Replace lines of file in same format as used before
    grouped = packed.reshape(-1, 8)
    for i, group in enumerate(grouped):
        line = ("   " + " 0x%02x," * 8) % tuple(group)
        lines[start + i] = line

    # Write out modified source file
    open(source_file, 'w').writelines([line + "\n" for line in lines])
