/**
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

#ifndef _SBS_H
#define _SBS_H

#define VERSION "1.0"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifndef _BIG_ENDIAN
#define BSWAP16(x) ((u16)((((u16)(x))>>8)|(((u16)(x))<<8)))
#define BSWAP32(x) ((u32)((BSWAP16(((u32)(x))&0x0000FFFFU)<<16)|(BSWAP16(((u32)(x))>>16))))
#define BSWAP64(x) ((u64)((BSWAP32(((u64)(x))&0x00000000FFFFFFFFULL)<<32)|(BSWAP32(((u64)(x))>>32))))
#else
#define BSWAP16(x) (x)
#define BSWAP32(x) (x)
#define BSWAP64(x) (x)
#endif

#define DOSWAP16(x) (x) = BSWAP16(*(u16*)&(x))
#define DOSWAP32(x) (x) = BSWAP32(*(u32*)&(x))
#define DOSWAP64(x) (x) = BSWAP64(*(u64*)&(x))

#define U32_TO_CHARS(x) (char) ((x) >> 24), (char) ((x) >> 16), (char) ((x) >> 8), (char) (x)

#pragma pack(push,1)

// files are big-endian and must be byteswapped before using on a x86 system

typedef struct {
	u32 magic; // "SBKR"
	u32 u1;
	u32 b1;
	u32 b2;
	u32 b3;
	u32 bankKey;
	u32 b4;
	u32 count;
	u32 u2;
	u32 entryOffset;
	u32 metaOffset;
	u32 b5;
	u16 u3;
} sbr_header;

#define SBR_MAGIC 0x53424B52U

#define DOSWAP_SBR_HEADER(x) {\
	DOSWAP32((x).magic);\
	DOSWAP32((x).u1);\
	DOSWAP32((x).b1);\
	DOSWAP32((x).b2);\
	DOSWAP32((x).b3);\
	DOSWAP32((x).bankKey);\
	DOSWAP32((x).b4);\
	DOSWAP32((x).count);\
	DOSWAP32((x).u2);\
	DOSWAP32((x).entryOffset);\
	DOSWAP32((x).metaOffset);\
	DOSWAP32((x).b5);\
	DOSWAP16((x).u3);\
}

typedef struct {
	u32 snr1;
	u16 u1;
	u32 sns1;
	u16 u2;
} sbr_metadata;

#define DOSWAP_SBR_METADATA(x) {\
	DOSWAP32((x).snr1);\
	DOSWAP16((x).u1);\
	DOSWAP32((x).sns1);\
	DOSWAP16((x).u2);\
}

typedef struct {
	u32 id;
	u16 count;
	u32 offset;
} sbr_entry;

#define DOSWAP_SBR_ENTRY(x) {\
	DOSWAP32((x).id);\
	DOSWAP16((x).count);\
	DOSWAP32((x).offset);\
}

typedef struct {
	u16 id; // 0 for stream metadata, 1 for stream offset (in sbs file)
	u32 offset;
} sbr_entry_metadata;

#define DOSWAP_SBR_ENTRY_METADATA(x) {\
	DOSWAP16((x).id);\
	DOSWAP32((x).offset);\
}

typedef struct {
	u8 codec;
	u8 channels;
	u16 freq;
	u32 samples; // high byte always 0x40
} sbr_stream_metadata;

#define DOSWAP_SBR_STREAM_METADATA(x) {\
	DOSWAP16((x).freq);\
	DOSWAP32((x).samples);\
}

typedef struct {
	u32 offset;
	u32 u1; // possibly size in other codec formats?
} sbr_stream_offset;

#define DOSWAP_SBR_STREAM_OFFSET(x) {\
	DOSWAP32((x).offset);\
	DOSWAP32((x).u1);\
}

typedef struct {
	u32 u1;
	u32 u2;
	u32 magic; // must be 0x20000000, at least for ToWav to work
	u32 u3;
	u8 codec;
	u8 channels;
	u16 freq;
	u32 samples;
	u32 u5;
	u32 u6;
} snu_header;

#define DOSWAP_SNU_HEADER(x) {\
	DOSWAP32((x).u1);\
	DOSWAP32((x).u2);\
	DOSWAP32((x).magic);\
	DOSWAP32((x).u3);\
	DOSWAP16((x).freq);\
	DOSWAP32((x).samples);\
	DOSWAP32((x).u5);\
	DOSWAP32((x).u6);\
}

#pragma pack(pop)

#endif
