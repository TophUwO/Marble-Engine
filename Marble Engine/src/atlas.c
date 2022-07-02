#pragma once

#include <atlas.h>
#include <color.h>
#include <error.h>


int Marble_Atlas_Create(int iType, void **ptrpAtlas) {
	static struct Marble_Internal_AtlasTypeTable { int iType; size_t stSize; } const gl_saAtlasTypeTable[] = {
		{ Marble_AtlasType_Unknown, sizeof(Marble_Atlas)      },

		{ Marble_AtlasType_Color,   sizeof(Marble_ColorAtlas) }
	};

	if (*ptrpAtlas = malloc(gl_saAtlasTypeTable[iType].stSize)) {
		extern int Marble_ColorAtlas_Internal_Create(Marble_ColorAtlas **ptrpColorAtlas);

		switch (((Marble_Atlas *)*ptrpAtlas)->iAtlasType = iType) {		
			case Marble_AtlasType_Color: return Marble_ColorAtlas_Internal_Create((Marble_ColorAtlas **)ptrpAtlas);
		}

		return Marble_ErrorCode_AtlasType;
	}

	return Marble_ErrorCode_MemoryAllocation;
}

void Marble_Atlas_Destroy(void **ptrpAtlas) {
	extern void Marble_ColorAtlas_Internal_Destroy(Marble_ColorAtlas **ptrpColorAtlas);

	if (ptrpAtlas && *ptrpAtlas) {
		switch (((Marble_Atlas *)*ptrpAtlas)->iAtlasType) {
			case Marble_AtlasType_Color: Marble_ColorAtlas_Internal_Destroy((Marble_ColorAtlas **)ptrpAtlas);
		}

		free(*ptrpAtlas);
		*ptrpAtlas = NULL;
	}
}


#pragma region Marble_ColorAtlas
static void Marble_ColorAtlas_Internal_DestroyColorEntry(void **ptrpColorEntry) {
	free(*ptrpColorEntry);
	*ptrpColorEntry = NULL;
}

static int Marble_ColorAtlas_Internal_Create(Marble_ColorAtlas **ptrpColorAtlas) {
	int iErrorCode = Marble_ErrorCode_Ok;
	if (iErrorCode = Marble_Util_Vector_Create(&(*ptrpColorAtlas)->sColorTable, 32, (void (*)(void **))&Marble_ColorAtlas_Internal_DestroyColorEntry)) {
		Marble_Atlas_Destroy(ptrpColorAtlas);

		return iErrorCode;
	}

	memset(&(*ptrpColorAtlas)->sHead, 0, sizeof(struct Marble_Internal_ColorAtlasHead));

	return iErrorCode;
}

static void Marble_ColorAtlas_Internal_Destroy(Marble_ColorAtlas **ptrpColorAtlas) {
	Marble_Util_Vector_Destroy(&(*ptrpColorAtlas)->sColorTable);
}

static _Bool Marble_ColorAtlas_Internal_ValidateHead(struct Marble_Internal_ColorAtlasHead *sHead) {
	static union { 
		DWORD dwDWORD; 
		CHAR const caString[sizeof(DWORD)]; 
	} const gl_uColorAtlasMagic = { .caString = { 'm', 'b', 'c', 'a' } };


}


int Marble_ColorAtlas_LoadFromFile(Marble_ColorAtlas *sAtlas, TCHAR const *strPath) {
	int iErrorCode = Marble_ErrorCode_Ok;

	Marble_Util_FileStream *sFile = NULL;
	if (iErrorCode = Marble_Util_FileStream_Open(TEXT("__test__.mbca"), Marble_Util_StreamPerm_Read, &sFile))
		return iErrorCode;

	Marble_IfError(
		iErrorCode = Marble_Util_FileStream_ReadSize(sFile, sizeof(struct Marble_Internal_ColorAtlasHead), &sAtlas->sHead),
		Marble_ErrorCode_Ok, 
		goto ON_ERROR
	);
	Marble_IfError(
		Marble_ColorAtlas_Internal_ValidateHead(&sAtlas->sHead), 
		TRUE, {
			iErrorCode = Marble_ErrorCode_HeadValidation; 

			goto ON_ERROR;
		}
	);
	Marble_IfError(
		iErrorCode = Marble_Util_FileStream_Goto(sFile, sAtlas->sHead.stBeginOfData),
		Marble_ErrorCode_Ok, 
		goto ON_ERROR
	);

	size_t const stEntrySize = Marble_Color_GetColorSizeByFormat(sAtlas->sHead.iColorFormat);
	for (size_t stIndex = 0; stIndex < sAtlas->sHead.stNumOfEntries; stIndex++) {
		void *ptrColorEntry = NULL;
		if (!(ptrColorEntry = malloc(stEntrySize))) {
			iErrorCode = Marble_ErrorCode_MemoryAllocation;

			goto ON_ERROR;
		}

		Marble_IfError(
			iErrorCode = Marble_Util_FileStream_ReadSize(sFile, stEntrySize, ptrColorEntry),
			Marble_ErrorCode_Ok, {
				Marble_Util_Vector_Clear(&sAtlas->sColorTable, TRUE);

				goto ON_ERROR;
			}
		);

		Marble_Util_Vector_PushBack(sAtlas->sColorTable, ptrColorEntry);
	}

ON_ERROR:
	Marble_Util_FileStream_Destroy(&sFile);

	return iErrorCode;
}

int Marble_ColorAtlas_GetColorByIndex(Marble_ColorAtlas *sAtlas, size_t stIndex) {

}
#pragma endregion


