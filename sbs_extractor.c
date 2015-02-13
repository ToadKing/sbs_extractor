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

int make_file(sbr_entry *entry, sbr_stream_metadata *meta, sbr_stream_offset *stream_offset)
{
	char fname[1024];
	FILE *sout = NULL;
	snu_header shead = {0};
	u8 buffer[0x10000];
	u32 numsamples = 0;
	int last = 0;
	int r = 0;

	if (fseek(sbs, stream_offset->offset, SEEK_SET))
	{
		printf("unexpected end of file (0x%08X)\n", stream_offset->offset);
		r = 7;
		goto error;
	}

	shead.u1 = 0;
	shead.u2 = 0;
	shead.magic = 0x20000000U;
	if (meta)
	{
		shead.u3 = 0;
		shead.codec = meta->codec;
		shead.channels = meta->channels;
		shead.freq = meta->freq;
		shead.samples = meta->samples;
	}
	else
	{
		if(fread(&shead.u3, 12, 1, sbs) != 1)
		{
			printf("unexpected end of file (0x%08X)\n", (u32) ftell(sbs));
			r = 12;
			goto error;
		}

		DOSWAP32(shead.u3);
		DOSWAP16(shead.freq);
		DOSWAP32(shead.samples);
	}
	shead.u5 = 0;
	shead.u6 = 0;

	if (shead.codec != 4 && shead.codec != 25)
	{
		printf("error: tool can currently only handle XAS audio (this is 0x%02X)\n", meta->codec);
		r = 5;
		goto error;
	}
	sprintf(fname, "%s/0x%08X.snu", name, entry->id);
	printf("saving %s\n", fname);

	sout = fopen(fname, "wb");

	if (sout == NULL)
	{
		printf("failed to open file %s for writing\n", fname);
		r = 6;
		goto error;
	}

	DOSWAP_SNU_HEADER(shead);
	fwrite(&shead, sizeof(shead), 1, sout);
	DOSWAP_SNU_HEADER(shead);

	for(;;)
	{
		struct {
			u16 h1;
			u16 h2;
			u32 samples;
		} h = {0};

		fread(&h, sizeof(h), 1, sbs);

		DOSWAP16(h.h1);
		DOSWAP16(h.h2);
		DOSWAP32(h.samples);

		if (h.h2 < 8)
		{
			h.samples = 0;
		}

		switch(h.h1)
		{
		case 0x8000:
		case 0x4500:
			last = 1;
		case 0x0000:
		case 0x4400:
			fseek(sbs, -8, SEEK_CUR);
			if (h.h2 == 0 || fread(buffer, h.h2, 1, sbs) != 1)
			{
				printf("unexpected end of file (0x%08X)\n", (u32) ftell(sbs));
				r = 8;
				goto error;
			}
			fwrite(buffer, h.h2, 1, sout);
			break;
		default:
			printf("unknown stream info (0x%04X)\n", h.h1);
			r = 9;
			goto error;
		}

		numsamples += h.samples;

		if (last)
		{
			break;
		}
	}

	if (numsamples != (shead.samples & ~0x40000000U))
	{
		printf("sample count mismatch (got %u, expected %u)", numsamples, (shead.samples & ~0x40000000U));
		r = 10;
		goto error;
	}

	error:

	if (sout != NULL)
	{
		fclose(sout);
	}

	return r;
}

int process_entry(sbr_entry *entry, int metas[], int meta_count)
{
	u16 i;
	int curr;
	int got_stream_offset = 0, got_stream_meta = 0;
	sbr_stream_metadata stream_data = {0};
	sbr_stream_offset stream_offset = {0};
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
		fprintf(out, " id:     0x%04X\n", meta.id);
		fprintf(out, " offset: 0x%08X\n", meta.offset);

		if (meta.id >= meta_count)
		{
			printf("metadata entry too high (%d)\n", meta.id);
			return 11;
		}

		if (metas[meta.id] == 1)
		{
			int c = ftell(f);
			fseek(f, meta.offset, SEEK_SET);
			fread(&stream_data, sizeof(stream_data), 1, f);
			DOSWAP_SBR_STREAM_METADATA(stream_data);
			fprintf(out, "  codec:    0x%02X\n", stream_data.codec);
			fprintf(out, "  channels: 0x%02X\n", stream_data.channels);
			fprintf(out, "  freq:     0x%04X\n", stream_data.freq);
			fprintf(out, "  samples:  0x%08X (%u, ~%.5fs)\n", stream_data.samples, (stream_data.samples & ~0x40000000U), (stream_data.freq) ? (float) (stream_data.samples & ~0x40000000U) / stream_data.freq : 0.0f);
			fseek(f, c, SEEK_SET);
			got_stream_meta = 1;
		}
		else if (metas[meta.id] == 2)
		{
			int c = ftell(f);
			fseek(f, meta.offset, SEEK_SET);
			fread(&stream_offset, sizeof(stream_offset), 1, f);
			DOSWAP_SBR_STREAM_OFFSET(stream_offset);
			fprintf(out, "  offset:   0x%08X\n", stream_offset.offset);
			fseek(f, c, SEEK_SET);
			got_stream_offset = 1;
		}
		else
		{
			printf("unknown metadata entry (%u)\n", meta.id);
			return 2;
		}
	}

	if (!got_stream_offset)
	{
		printf("did not get stream info needed\n");
		return 3;
	}

	r = make_file(entry, got_stream_meta ? &stream_data : NULL, &stream_offset);

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
	sbr_header head = {0};
	sbr_metadata meta = {0};
	int meta_order[256] = {0};
	int meta_count = 0;
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

	mkdir(name, 0777);
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

	is_be = 1;
	DOSWAP32BE(head.magic);

	if (head.magic == SBR_MAGIC_BE)
	{
		is_be = 1;
	}
	else if (head.magic == SBR_MAGIC_LE)
	{
		is_be = 0;
	}
	else
	{
		printf("file unknown format\n");
		printf("got 0x%08X\n", head.magic);
		goto error;
	}

	DOSWAP_SBR_HEADER(head);

	fprintf(out, "HEADER:\n");
	fprintf(out, "magic:       \"%c%c%c%c\"\n", U32_TO_CHARS(head.magic));
	fprintf(out, "u1:          0x%08X\n", head.u1);
	fprintf(out, "b1:          0x%08X\n", head.b1);
	fprintf(out, "b2:          0x%08X\n", head.b2);
	fprintf(out, "b3:          0x%08X\n", head.b3);
	fprintf(out, "bankKey:     0x%08X\n", head.bankKey);
	fprintf(out, "b4:          0x%08X\n", head.b4);
	fprintf(out, "count:       0x%08X\n", head.count);
	fprintf(out, "u2:          0x%08X\n", head.u2);
	fprintf(out, "entryOffset: 0x%08X\n", head.entryOffset);
	fprintf(out, "metaOffset:  0x%08X\n", head.metaOffset);
	fprintf(out, "b5:          0x%08X\n", head.b5);
	fprintf(out, "u3:          0x%04X\n", head.u3);

	fseek(f, head.metaOffset, SEEK_SET);
	entries = calloc(head.count, sizeof(*entries));

	fprintf(out, "\nMETADATA:\n");

	for (;;)
	{
		fread(&meta, sizeof(meta), 1, f);

		DOSWAP_SBR_METADATA(meta);

		fprintf(out, "id:   \"%c%c%c%c\"\n", U32_TO_CHARS(meta.id));
		fprintf(out, "u1:   0x%04X\n\n", meta.u1);

		if (meta.id == 0x534E5231) // SNR1
		{
			meta_order[meta_count++] = 1;
		}
		else if (meta.id == 0x534E5331 || // SNS1
			meta.id == SBR_MAGIC_LE)
		{
			meta_order[meta_count++] = 2;
			break;
		}
		else
		{
			printf("unknown meta info \"%c%c%c%c\"\n", U32_TO_CHARS(meta.id));
			goto error;
		}
	}

	fprintf(out, "ENTRY:\n");

	fseek(f, head.entryOffset, SEEK_SET);

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
		fprintf(out, "count:  0x%04X\n", entries[i].count);
		fprintf(out, "offset: 0x%08X\n", entries[i].offset);

		r2 = process_entry(&entries[i], meta_order, meta_count);
		if (r2)
		{
			printf("failed: %d\n", r2);
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

	if (r)
	{
		printf("please issue a bug report at https://github.com/ToadKing/sbs_extractor/issues\n");
		#ifdef _WIN32
		fflush(stdout);
		system("pause");
		#endif
	}

	return r;
}
