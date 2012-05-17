# fixnames.py - rename output of sbs_extract based off an asset XML file
#
# Copyright (c) 2012 Michael Lelli
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import xml.dom.minidom
import os
import sys

def parseNode(n):
	usednames = dict()
	renamed = 0
	total = 0
	for e in n.getElementsByTagName("enumerator"):
		total += 1
		name = e.getAttribute("name")
		append = ""

		if name in usednames:
			usednames[name] += 1
			append = "__{}".format(usednames[name])
		else:
			usednames[name] = 1

		filename = "0x{:08X}.wav".format(int(e.getAttribute("value"), 16))
		newname = "{}{}.wav".format(name.replace("RWAUDIOSB_COMMENTARY_P4_BOUNCE_DL_DATA_BOUNCEDATAAUDIO_DEV_SPEECH_SAMPLES_ENGLISH_TIM_KITZROW_MST_16B_48K_", "").replace("RWAUDIOSB_COMMENTARY_P4_BOUNCE_DL_DATA_BOUNCEDATAAUDIO_DEV_SPEECH_SAMPLES_ENGLISH_SILENCE_", ""), append)
		try:
			os.rename(filename, newname)
			print "{} -> {}".format(filename, newname)
			renamed += 1
		except:
			print "could not rename {} to {}: {}".format(filename, newname, sys.exc_info()[1])
	print
	print "{} of {} files renamed".format(renamed, total)

for p in xml.dom.minidom.parse(sys.argv[1]).getElementsByTagName("enum"):
	if p.getAttribute("name") == sys.argv[2]:
		parseNode(p)
