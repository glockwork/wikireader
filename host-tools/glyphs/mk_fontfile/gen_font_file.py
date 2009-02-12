#!/usr/bin/python
"""
 Generate a font file to be used on the target device

 Copyright (C) 2009 Daniel Mack <daniel@caiaq.de>
 Copyright (C) 2009 Holger Hans Peter Freyther <zecke@openmoko.org>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import gd
import os
import struct
import array
import sys

fontpath = ""
outfilename_default = "fontfile.gen"
fontmapfilename_default = "fontmap.map"

font_name_to_number = {}

def usage():
	print "Wikipedia reader font file generator"
	print "Usage: %s <fontpath> [<outfilename>] [<fontmapfilename>]" % (sys.argv[0])
	print "\t<fontpath> points to a tree of glyphs generated by ???"
	print "\t<outfilename> is the name of the output file name. Defaults to '%s'" % (outfilename_default)
	print "\t<fontmapfilename> is the name of the font map. Defaults  '%s'" % (fontmapfilename_default)
	sys.exit(1)

def gen_spacing_hints(fontid, glyphid):
	spacingpath = fontpath + "/" + fontid + "/spacing/"
	try:
		all = os.listdir(spacingpath)
	except:
		return ""

	filtered = array.array('I')
	spacings = ""

	for s in all:
		if s[:len(glyphid) + 1] == glyphid + "-":
			c = int(s[len(glyphid)+1:])
			filtered.append(c)

	for other in filtered:
		fname = spacingpath + "/" + glyphid + "-" + str(other) + "/spacing"
		try:
			f = open(fname, 'r')
			s = f.read()
			f.close()
			a = s.split(",");
			x = int(a[0])
			y = int(a[1])
			spacings += struct.pack("<hbb", other, x, y)
		except:
			print "unable to parse %s" % (fname)
			raise
			continue

	return spacings

def get_max(list):
	max = 0

	for e in list:
		if int(e) > max:
			max = int(e)

	return max

def gen_font(font_name):
	path = os.path.join(fontpath, font_name)
	glyphpath = path
	glyphlist = filter(lambda x: x != 'spacing', os.listdir(glyphpath))

	# the index table will take an unsigned int for each entry
	n_glyphs = get_max(glyphlist) + 1
	offsettable = [ 0 ] * n_glyphs
	offset = n_glyphs * 4
	out = ""
	print "font %s has %d glyphs" % (font_name, n_glyphs)

	for glyphid in glyphlist:
		imagefile = os.path.join(glyphpath, glyphid, "bitmap.png")
		spacing_hints = gen_spacing_hints(font_name, glyphid)

		try:
			try: 
				im = gd.image(imagefile)
				(w, h) = im.size()
			except:
				print "unable to open bitmap file >%s< using empty image" % (imagefile)
				w = 1
				h = int(open(os.path.join(glyphpath, glyphid, "advance_x")).read())
				im = gd.image((w,h))

			offsettable[int(glyphid)] = offset;

			bearing_path = os.path.join(glyphpath, glyphid, "bitmap_top_bearing")
			bearing = int(open(bearing_path).read())

			# a spacing hint is always 4 bytes ...
			n_spacing_hints = len(spacing_hints) / 4;
			header = struct.pack("<BBbI", w, h, bearing, n_spacing_hints)

			#print "%s/ %d,%d" % (font_name, w, h)

			bit = 0;
			outbyte = 0;
			offset += len(header);
			out += header

			for n in range (0, w * h):
				pixel = im.getPixel((n % w, n / w))
				bit = n % 8;
	
				(r, g, b) = im.colorComponents(pixel)
				color = (r + g + b) / 3

				if (color > 127):
					outbyte |= 1 << bit;
				
				if bit == 7:
					out += struct.pack("B", outbyte)
					outbyte = 0
					offset += 1
					bit = 0

			if bit > 0:
				out += struct.pack("B", outbyte)
				offset += 1

			# im.close()

		except:
			print "broken glyph... description >%s<" % (imagefile)
			continue

		out += spacing_hints
		offset += len(spacing_hints)

	table = ""
	for i in offsettable:
		table += struct.pack("<I", i)

	print "index to glyph 0 (font %s) is %d" % (font_name, offsettable[0])
	return struct.pack("<I", n_glyphs) + table + out

if (len(sys.argv) < 2):
	usage()

fontpath = sys.argv[1]

if (len(sys.argv) > 2):
	outfilename = sys.argv[2]
else:
	outfilename = outfilename_default

if (len(sys.argv) > 3):
	fontmapfilename = sys.argv[3]
else:
	fontmapfilename = fontmapfilename_default

try:
	fontlist = os.listdir(fontpath)
except:
	print "unable to open font path '%s'" % (fontpath)
	sys.exit(1)

# allocate the font offset table
fonttable = [ 0 ] * len(fontlist)
offset = len(fontlist) * 4
fontnum = 0
font_name_to_number = {}
out = ""

for _font in fontlist:
	font_name_to_number[_font] = fontnum
	f = gen_font(_font)
	out += f
	fonttable[fontnum] = offset;
	print "offset for font %d is %d" % (fontnum, offset)
	fontnum += 1
	offset += len(f)

outfile = open(outfilename, 'w')

# write the number of fonts in this file
outfile.write(struct.pack("<I", len(fontlist)))

for i in fonttable:
	outfile.write(struct.pack("<I", i))

outfile.write(out)
outfile.close()

fontmap = open(fontmapfilename, 'w')
for font in font_name_to_number.keys():
    print >> fontmap, "%s %d" % (font, font_name_to_number[font])
fontmap.close()

print "generated file >%s<, size %d + %d" % (outfilename, len(out), 4)

