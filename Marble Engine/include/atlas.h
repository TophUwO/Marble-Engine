#pragma once

#include <util.h>


enum Marble_AtlasType {
	Marble_AtlasType_Unknown,

	Marble_AtlasType_Color
};


typedef struct Marble_Atlas {
	int iAtlasType;
} Marble_Atlas;

typedef struct Marble_ColorAtlas {
	Marble_Atlas;
	struct Marble_Internal_ColorAtlasHead {
		DWORD  dwMagic;
		DWORD  dwMinVersion;
		CHAR   astrIdent[65];
		size_t stNumOfEntries;
		int    iColorFormat;
		size_t stBeginOfData;
	} sHead;

	Marble_Util_Vector *sColorTable;
} Marble_ColorAtlas;


extern int  Marble_Atlas_Create(int iAtlasType, void **ptrpAtlas);
extern void Marble_Atlas_Destroy(void **ptrpAtlas);

extern int        Marble_ColorAtlas_LoadFromFile(Marble_ColorAtlas *sAtlas, TCHAR const *strPath);
extern int inline Marble_ColorAtlas_GetColorByIndex(Marble_ColorAtlas *sAtlas, size_t stIndex);


