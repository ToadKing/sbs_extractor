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

FILE *f;
FILE *out;
FILE *sbs;
char name[1024];

int make_file(sbr_entry *entry, sbr_stream_metadata *meta, u32 stream_offset)
{
	char fname[1024];
	FILE *sout = NULL;
	snu_header shead;
	u8 buffer[0x10000];
	int last = 0;
	int r = 0;

	sprintf(fname, "%s/0x%08X.snu", name, entry->id);
	printf("saving %s\n", fname);

	sout = fopen(fname, "wb");

	if (sout == NULL)
	{
		printf("failed to open file %s for writing\n", fname);
		goto error;
	}

	shead.u1 = 0;
	shead.u2 = 0;
	shead.magic = 0x20000000U;
	shead.u3 = 0;
	shead.u4 = meta->u1;
	shead.freq = meta->freq;
	shead.u5 = meta->u2;
	shead.u6 = 0;
	shead.u7 = 0;

	DOSWAP_SNU_HEADER(shead);

	fwrite(&shead, sizeof(shead), 1, sout);

	if (fseek(sbs, stream_offset, SEEK_SET))
	{
		printf("unexpected end of file (0x%08X)\n", stream_offset);
		r = 4;
		goto error;
	}

	for(;;)
	{
		u16 h[2];

		if (last)
		{
			break;
		}

		if(fread(h, sizeof(h), 1, sbs) != 1)
		{
			printf("unexpected end of file (0x%08X)\n", (u32) ftell(sbs));
			r = 5;
			goto error;
		}

		DOSWAP16(h[0]);
		DOSWAP16(h[1]);

		switch(h[0])
		{
		case 0x8000:
			last = 1;
		case 0x0000:
			fseek(sbs, -4, SEEK_CUR);
			if (fread(buffer, h[1], 1, sbs) != 1)
			{
				printf("unexpected end of file (0x%08X)\n", (u32) ftell(sbs));
				r = 6;
				goto error;
			}
			fwrite(buffer, h[1], 1, sout);
			break;
		default:
			printf("unknown stream info (0x%02X)\n", h[0]);
			r = 7;
			goto error;
		}
	}

	error:

	if (sout != NULL)
	{
		fclose(sout);
	}

	return r;
}

int process_entry(sbr_entry *entry)
{
	u16 i;
	int curr;
	int got_stream_info = 0, got_stream_offset = 0;
	sbr_stream_metadata stream_data;
	u32 stream_offset;
	int r;

	curr = ftell(f);
	fseek(f, entry->offset, SEEK_SET);

	for (i = 0; i < entry->count; i++)
	{
		sbr_entry_metadata meta;
		if (fread(&meta, sizeof(meta), 1, f) != 1)
		{
			printf("unexpected end of file\n");
			return 1;
		}

		DOSWAP_SBR_ENTRY_METADATA(meta);
		fprintf(out, " id:     0x%04hX\n", meta.id);
		fprintf(out, " offset: 0x%08X\n", meta.offset);

		if (meta.id == 0)
		{
			int c = ftell(f);
			fseek(f, meta.offset, SEEK_SET);
			fread(&stream_data, sizeof(stream_data), 1, f);
			DOSWAP_SBR_STREAM_METADATA(stream_data);
			fprintf(out, "  u1:     0x%04X\n", stream_data.u1);
			fprintf(out, "  freq:   0x%04X\n", stream_data.freq);
			fprintf(out, "  u2:     0x%08X\n", stream_data.u2);
			fseek(f, c, SEEK_SET);
			got_stream_info = 1;
		}
		else if (meta.id == 1)
		{
			int c = ftell(f);
			fseek(f, meta.offset, SEEK_SET);
			fread(&stream_offset, sizeof(stream_offset), 1, f);
			DOSWAP32(stream_offset);
			fprintf(out, "  offset: 0x%08X\n", stream_offset);
			fseek(f, c, SEEK_SET);
			got_stream_offset = 1;
		}
		else
		{
			printf("unknown metadata entry\n");
			return 2;
		}
	}

	if (!(got_stream_info && got_stream_offset))
	{
		printf("did not get stream info needed\n");
		return 3;
	}

	r = make_file(entry, &stream_data, stream_offset);

	if (r)
	{
		return r;
	}

	fseek(f, curr, SEEK_SET);
	return 0;
}

int main(int argc, char *argv[])
{
	f = NULL;
	out = NULL;
	sbs = NULL;
	u32 i;
	sbr_header head;
	sbr_metadata meta;
	sbr_entry *entries = NULL;
	char sbrname[1024];
	char sbsname[1024];
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

	_mkdir(name);
	sprintf(sbsname, "%s.%s", name, "sbs");
	sbs = fopen(sbsname, "rb");

	if (sbs == NULL)
	{
		printf("file %s can't be opened\n", sbsname);
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
	entries = calloc(head.count, sizeof(*entries));

	fread(&meta, sizeof(meta), 1, f);

	DOSWAP_SBR_METADATA(meta);

	fprintf(out, "\nMETADATA:\n");
	fprintf(out, "snr1: \"%c%c%c%c\"\n", U32_TO_CHARS(meta.snr1));
	fprintf(out, "u1:   0x%04hX\n", meta.u1);
	fprintf(out, "sns1: \"%c%c%c%c\"\n", U32_TO_CHARS(meta.sns1));
	fprintf(out, "u2:   0x%04hX\n", meta.u2);

	DOSWAP_SBR_METADATA(meta);

	fprintf(out, "\nENTRY:\n");

	for(i = 0; i < head.count; i++)
	{
		int r2;
		if (fread(&(entries[i]), sizeof(*entries), 1, f) != 1)
		{
			printf("unexpected end of file\n");
			goto error;
		}

		DOSWAP_SBR_ENTRY(entries[i]);

		fprintf(out, "\nid:     0x%08X\n", entries[i].id);
		fprintf(out, "count:  0x%04hX\n", entries[i].count);
		fprintf(out, "offset: 0x%08X\n", entries[i].offset);

		r2 = process_entry(&entries[i]);
		if (r2)
		{
			printf("failed: %u", r2);
			goto error;
		}
	}

	goto end;

	error:
	r = 1;

	end:
	free(entries);

	if (f != NULL)
	{
		fclose(f);
	}

	if (out != NULL)
	{
		fclose(out);
	}

	if (sbs != NULL)
	{
		fclose(sbs);
	}

	return r;
}