import xml.dom.minidom
import os
import sys

def parseNode(n):
	enums = n.getElementsByTagName("enumerator");
	for e in enums:
		name = e.getAttribute("name")
		append = ""

		if name in usednames:
			usednames[name] += 1
			append = "__%d" % usednames[name]
		else:
			usednames[name] = 1

		filename = "0x%08X.snu" % int(e.getAttribute("value"), 16)
		newname = "%s%s.snu" % (name.replace("RWAUDIOSB_COMMENTARY_P4_BOUNCE_DL_DATA_BOUNCEDATAAUDIO_DEV_SPEECH_SAMPLES_ENGLISH_TIM_KITZROW_MST_16B_48K_", "").replace("RWAUDIOSB_COMMENTARY_P4_BOUNCE_DL_DATA_BOUNCEDATAAUDIO_DEV_SPEECH_SAMPLES_ENGLISH_SILENCE_", ""), append)
		print "%s -> %s" %(filename, newname)
		os.rename(filename, newname)

dom = xml.dom.minidom.parse(sys.argv[1])
usednames = dict()
parents = dom.getElementsByTagName("enum")

for p in parents:
	if p.getAttribute("name") == sys.argv[2]:
		parseNode(p)
