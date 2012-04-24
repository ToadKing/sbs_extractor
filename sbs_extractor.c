/**
 * sbs_extractor.c - a simple extractor of .sbs and .sbr file combos found in
 * EA Sports games
 *
 * Copyright (c) 2012 Michael Lelli
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sbs.h"

int main(int argc, char *argv[])
{
	FILE *f = NULL;
	FILE *out = NULL;
	u32 i, c;
	sbr_header head;
	sbr_metadata meta;
	sbr_entry1 *entries1 = NULL;
	sbr_entry2 *entries2 = NULL;
	sbr_entry3 *entries3 = NULL;
	char name[1024];
	char sbrname[1024];
	int r = 0;

	if (argc > 3 || argc < 2)
	{
		printf("sbs_extractor " VERSION "\n"
		       "usage: %s file.sbs/sbr [log.txt]\n", argv[0]);
		goto error;
	}

	memcpy(name, argv[1], strlen(argv[1]) + 1);
	name[strrchr(name, '.') - name] = 0;

	sprintf(sbrname, "%s.%s", name, "sbr");
	f = fopen(sbrname, "rb");

	if (f == NULL)
	{
		printf("file %s can't be opened\n", sbrname);
		goto error;
	}

	if (argc == 2)
	{
		char logname[1024];
		sprintf(logname, "%s.txt", name);
		out = fopen(logname, "w");
	}
	else
	{
		out = fopen(argv[2], "w");
	}

	if (out == NULL)
	{
		printf("log %s can't be opened for writing\n", argv[2]);
		goto error;
	}

	fread(&head, sizeof(head), 1, f);

	DOSWAP_SBR_HEADER(head);

	if (head.magic != SBR_MAGIC)
	{
		printf("file incorrect format\n");
		printf("expected 0x%08X, got 0x%08X\n", SBR_MAGIC, head.magic);
		goto error;
	}

	fprintf(out, "HEADER:\n");
	fprintf(out, "magic:   \"%c%c%c%c\"\n", U32_TO_CHARS(head.magic));
	fprintf(out, "u1:      0x%08X\n", head.u1);
	fprintf(out, "b1:      0x%08X\n", head.b1);
	fprintf(out, "b2:      0x%08X\n", head.b2);
	fprintf(out, "b3:      0x%08X\n", head.b3);
	fprintf(out, "bankKey: 0x%08X\n", head.bankKey);
	fprintf(out, "b4:      0x%08X\n", head.b4);
	fprintf(out, "count:   0x%08X\n", head.count);
	fprintf(out, "u2:      0x%08X\n", head.u2);
	fprintf(out, "u3:      0x%08X\n", head.u3);
	fprintf(out, "offset:  0x%08X\n", head.offset);
	fprintf(out, "b5:      0x%08X\n", head.b5);
	fprintf(out, "u4:      0x%04hX\n", head.u4);

	fseek(f, head.offset, SEEK_SET);
	entries1 = calloc(head.count, sizeof(*entries1));

	fread(&meta, sizeof(meta), 1, f);

	DOSWAP_SBR_METADATA(meta);

	fprintf(out, "\nMETADATA:\n");
	fprintf(out, "snr1: \"%c%c%c%c\"\n", U32_TO_CHARS(meta.snr1));
	fprintf(out, "u1:   0x%04hX\n", meta.u1);
	fprintf(out, "sns1: \"%c%c%c%c\"\n", U32_TO_CHARS(meta.sns1));
	fprintf(out, "u2:   0x%04hX\n", meta.u2);

	DOSWAP_SBR_METADATA(meta);

	fprintf(out, "\nENTRY1:\n");

	for(i = 0; i < head.count; i++)
	{
		if (fread(&(entries1[i]), sizeof(*entries1), 1, f) != 1)
		{
			printf("unexpected end of file\n");
			goto error;
		}

		DOSWAP_SBR_ENTRY1(entries1[i]);

		fprintf(out, "\nid: 0x%08X\n", entries1[i].id);
		fprintf(out, "u1: 0x%04hX\n", entries1[i].u1);
		fprintf(out, "u2: 0x%08X\n", entries1[i].u2);

		if (entries1[i].u1 != 0x0002) printf("weird value at 0x%08X\n", entries1[i].id);
	}

	entries2 = calloc(head.count, sizeof(*entries2));

	for(i = 0; i < head.count; i++)
	{
		if (fread(&(entries2[i]), sizeof(*entries2), 1, f) != 1)
		{
			printf("unexpected end of file\n");
			goto error;
		}

		DOSWAP_SBR_ENTRY2(entries2[i]);

		if (!entries2[i].u1 && !entries2[i].u2 && !entries2[i].u3 && !entries2[i].u4) break;
	}

	fprintf(out, "\nENTRY2 COUNT: 0x%08X\n", i - 1);
	// TODO: calculate this instead of using the last non-null value as a terminator, if possible
	c = i - 1;
	entries3 = calloc(head.count, sizeof(*entries3));
	fprintf(out, "\nENTRY2/ENTRY3:\n");

	for(i = 0; i < c; i++)
	{
		if (fread(&(entries3[i]), sizeof(*entries3), 1, f) != 1)
		{
			printf("unexpected end of file (0x%08X)\n", i);
			goto error;
		}

		DOSWAP_SBR_ENTRY3(entries3[i]);

		fprintf(out, "\ne2.u1:     0x%04hX\n", entries2[i].u1);
		fprintf(out, "e2.u2:     0x%08X\n", entries2[i].u2);
		fprintf(out, "e2.u3:     0x%04hX\n", entries2[i].u3);
		fprintf(out, "e2.u4:     0x%08X\n", entries2[i].u4);

		fprintf(out, "e3.b1:     0x%08X\n", entries3[i].b1);
		fprintf(out, "e3.u1:     0x%04hX\n", entries3[i].u1);
		fprintf(out, "e3.freq:   %hu\n", entries3[i].freq);
		fprintf(out, "e3.u2:     0x%08X\n", entries3[i].u2);
		fprintf(out, "e3.offset: 0x%08X\n", entries3[i].offset);

		fseek(f, 16, SEEK_CUR);
	}

	goto end;

	error:
	r = 1;

	end:
	free(entries1);
	free(entries2);
	free(entries3);

	if (f != NULL)
	{
		fclose(f);
	}

	if (out != NULL)
	{
		fclose(out);
	}
	return r;
}