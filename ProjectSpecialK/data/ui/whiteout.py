import os, sys
from pathlib import Path
from PIL import Image

scriptdir = os.path.dirname(os.path.abspath(__file__))

im = Image.open(sys.argv[1])
if im.mode == 'RGBA':
	width, height = im.size
	for y in range(height):
		for x in range(width):
			r, g, b, a = im.getpixel((x, y))
			im.putpixel((x, y), (255, 255, 255, a))
	im.save(sys.argv[1], "PNG")

