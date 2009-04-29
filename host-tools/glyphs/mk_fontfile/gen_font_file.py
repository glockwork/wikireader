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
import optparse
import os
import struct
import sys
import spacing

# Fontname to Number (Int) map and
# Font -> Glyph -> Integer mapping
font_name_to_number = {}
glyph_remap = {}


def parse():
	parser = optparse.OptionParser(version = "Wikipedia reader font file generator",
					usage = """%prog [options] input_file""")
	parser.add_option("-f", "--fontpath", help = "points to a tree of glyphs generated by ???",
			action = "store", dest = "fontpath")
	parser.add_option("-o", "--output", help = "is the name of the output file name.",
			action = "store", dest = "output", default = "fontfile.gen")
	parser.add_option("-m", "--fontmapfile", help = "The to be generated font name to number map",
			action = "store", dest = "fontmap", default = "fontmap.map");
	parser.add_option("-d", "--default", help = "The font to be used as font 0. A path to that directory with glyphs.",
			action = "store", dest = "default_font", default = "Liberation_Sans_9")
	parser.add_option("-g", "--glyphmapfile", help = "Mapping of font/glyph to font/number",
			action = "store", dest = "glyphmap", default = "glyphmap.map")
	(opts, args) = parser.parse_args(sys.argv)

	if not opts.fontpath:
	    print "No fontpath specified"
	    sys.exit(-1)
	else:
	    return opts, args

def write_output(fonttable, font_name_to_number, glyph_remap):
	global opts
	outfile = open(opts.output, 'w')

	# write the number of fonts in this file
	outfile.write(struct.pack("<I", len(fonttable)))

	for i in fonttable:
		outfile.write(struct.pack("<I", i))

	outfile.write(out)
	outfile.close()

	fontmap = open(opts.fontmap, 'w')
	for font in font_name_to_number.keys():
		print >> fontmap, "%s %d" % (font, font_name_to_number[font])
	fontmap.close()

	# Write out the glyph remapping
	glyphmap = open(opts.glyphmap, 'w')
	for font in glyph_remap.keys():
		for glyph in glyph_remap[font].keys():
			print >> glyphmap, "%d %s %d" % (font_name_to_number[font], glyph, glyph_remap[font][glyph])
	glyphmap.close()
	print "generated file >%s<, size %d + %d" % (opts.output, len(out), 4)

def get_mapped_glyph(font_name, glyphid):
	"""
	Map glyphid to another integer. We attempt to have a quite
	compact glyph structure and start with glyph number 0 is
	every font. Only for our default font we will have a direct
	mapping.
	"""
	global glyph_remap

	if opts.default_font == font_name:
		print "Default mapping %d" % glyphid
		return glyphid

	try:
		font_section = glyph_remap[font_name]
	except KeyError:
		glyph_remap[font_name] = {}
		font_section = glyph_remap[font_name]

	try:
		return font_section[glyphid]
	except:
		font_section[glyphid] = len(font_section)
		return font_section[glyphid]

def gen_spacing_hints(fontname, spacing_file, glyphid):
	try:
		other_glyphs = spacing_file[glyphid]
	except KeyError:
		# Okay, we don't have anything right to this glyph
		return ""

	spacings = ""
	for other in other_glyphs:
		assert other.left == int(glyphid)
		spacings += struct.pack("<hbb",
				get_mapped_glyph(fontname, other.right),
				other.x, other.y)

	return spacings

def get_max(list):
	max = 0

	for e in list:
		if int(e) > max:
			max = int(e)

	return max

def gen_font(font_name, glyphpath):
	glyphlist = filter(lambda x: x != 'spacing', os.listdir(glyphpath))

	spacing_file = spacing.load(open(
				os.path.join(glyphpath, "spacing", "spacing-file")))

	# the index table will take an unsigned int for each entry
	if font_name == opts.default_font:
		n_glyphs = get_max(glyphlist) + 1
	else:
		n_glyphs = len(glyphlist) + 1

	offsettable = [ 0 ] * n_glyphs
	offset = n_glyphs * 4
	out = ""
	print "font %s has %d glyphs" % (font_name, n_glyphs)

	for glyphid in glyphlist:
		imagefile = os.path.join(glyphpath, glyphid, "bitmap.png")
		if font_name == opts.default_font:
			spacing_hints = ""
		else:
			spacing_hints = gen_spacing_hints(font_name, spacing_file, glyphid)

		try:
			try: 
				im = gd.image(imagefile)
				(w, h) = im.size()
			except:
				print "unable to open bitmap file >%s< using empty image" % (imagefile)
				w = 1
				h = int(open(os.path.join(glyphpath, glyphid, "advance_x")).read())
				im = gd.image((w,h))

			offsettable[get_mapped_glyph(font_name, int(glyphid))] = offset;

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
					outbyte |= 1 << 7 - bit;
				
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

opts, args = parse()
print opts.fontpath, opts.output, opts.fontmap, opts.default_font

try:
	fontlist = os.listdir(opts.fontpath)
except:
	print "unable to open font path '%s'" % (opts.fontpath)
	sys.exit(1)

# allocate the font offset table
fonttable = [ 0 ] * (len(fontlist) + 1)
offset = len(fonttable) * 4
fontnum = 1
font_name_to_number = {}
out = ""

# special case the default font... and then forget about it
current_offset = offset
f = gen_font(opts.default_font, opts.default_font)
out += f
offset += len(f)
fonttable[0] = current_offset
# Forget so the glyph mapping gets effective
opts.default_font = None

for _font in fontlist:
	current_offset = offset
	f = gen_font(_font, os.path.join(opts.fontpath, _font))
	out += f
	offset += len(f)

	font_name_to_number[_font] = fontnum
	fonttable[fontnum] = current_offset;
	fontnum += 1
	print "offset for font %d is %d" % (font_name_to_number[_font], current_offset)

write_output(fonttable, font_name_to_number, glyph_remap)
