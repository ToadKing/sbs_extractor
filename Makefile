all:
	gcc -O3 -Wall -Wextra -Wno-missing-field-initializers -std=c99 sbs_extractor.c -o sbs_extractor

debug:
	gcc -O0 -g -Wall -Wextra -Wno-missing-field-initializers -std=c99 sbs_extractor.c -o sbs_extractor
