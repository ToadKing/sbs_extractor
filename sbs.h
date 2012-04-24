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

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define BSWAP16(x) ((u16)((((u16)(x))>>8)|(((u16)(x))<<8)))
#define BSWAP32(x) ((u32)((BSWAP16(((u32)(x))&0x0000FFFF)<<16)|(BSWAP16(((u32)(x))>>16))))
#define BSWAP64(x) ((u64)((BSWAP32(((u64)(x))&0x00000000FFFFFFFFULL)<<32)|(BSWAP32(((u64)(x))>>32))))

#define DOSWAP16(x) (x) = BSWAP16(x)
#define DOSWAP32(x) (x) = BSWAP32(x)
#define DOSWAP64(x) (x) = BSWAP64(x)

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
	u32 u3;
	u32 offset;
	u32 b5;
	u16 u4;
} sbr_header;

#define SBR_MAGIC 0x53424B52

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
	DOSWAP32((x).u3);\
	DOSWAP32((x).offset);\
	DOSWAP32((x).b5);\
	DOSWAP16((x).u4);\
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
	u16 u1;
	u32 u2;
} sbr_entry1;

#define DOSWAP_SBR_ENTRY1(x) {\
	DOSWAP32((x).id);\
	DOSWAP16((x).u1);\
	DOSWAP32((x).u2);\
}

typedef struct {
	u16 u1;
	u32 u2;
	u16 u3;
	u32 u4;
} sbr_entry2;

#define DOSWAP_SBR_ENTRY2(x) {\
	DOSWAP16((x).u1);\
	DOSWAP32((x).u2);\
	DOSWAP16((x).u3);\
	DOSWAP32((x).u4);\
}

typedef struct {
	u16 u1;
	u16 freq;
	u32 u2;
	u32 offset;
} sbr_entry3;

#define DOSWAP_SBR_ENTRY3(x) {\
	DOSWAP16((x).u1);\
	DOSWAP16((x).freq);\
	DOSWAP32((x).u2);\
	DOSWAP32((x).offset);\
}

#pragma pack(pop)

#endif
