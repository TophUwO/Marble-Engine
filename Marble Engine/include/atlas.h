#pragma once

#include <util.h>
#include <asset.h>


typedef struct Marble_Atlas {
	Marble_Asset;
	int iAtlasType;
} Marble_Atlas;

typedef struct Marble_ColorAtlas {
	Marble_Atlas;
	struct Marble_Internal_ColorAtlasHead {
		DWORD  dwMagic;
		DWORD  dwMinVersion;
		CHAR   astrIdent[64];
		size_t stNumOfEntries;
		int    iColorFormat;
		size_t stBeginOfData;
	} sHead;
	Marble_Util_Vector *sColorTable;
} Marble_ColorAtlas;


