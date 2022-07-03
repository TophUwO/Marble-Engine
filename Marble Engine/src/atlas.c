#pragma once

#include <application.h>


#pragma region Marble_ColorAtlas
static void Marble_ColorAtlas_Internal_DestroyColorEntry(void **ptrpColorEntry) {
	free(*ptrpColorEntry);
	*ptrpColorEntry = NULL;
}

static int Marble_ColorAtlas_Internal_Create(Marble_ColorAtlas **ptrpColorAtlas) {
	void Marble_Atlas_Destroy(void **ptrpAtlas);

	if (!(*ptrpColorAtlas = malloc(sizeof(**ptrpColorAtlas))))
		return Marble_ErrorCode_MemoryAllocation;

	int iErrorCode = Marble_ErrorCode_Ok;
	if (iErrorCode = Marble_Util_Vector_Create(&(*ptrpColorAtlas)->sColorTable, 32, (void (*)(void **))&Marble_ColorAtlas_Internal_DestroyColorEntry)) {
		Marble_Atlas_Destroy(ptrpColorAtlas);

		return iErrorCode;
	}
	memset(&(*ptrpColorAtlas)->sHead, 0, sizeof(struct Marble_Internal_ColorAtlasHead));

	(*ptrpColorAtlas)->iAtlasType = Marble_AtlasType_ColorAtlas;
	return iErrorCode;
}

static void Marble_ColorAtlas_Internal_Destroy(Marble_ColorAtlas **ptrpColorAtlas) {
	Marble_Util_Vector_Destroy(&(*ptrpColorAtlas)->sColorTable);
}

static _Bool Marble_ColorAtlas_Internal_ValidateHead(struct Marble_Internal_ColorAtlasHead *sHead) {
	static CHAR   const gl_acaString[sizeof(DWORD)] = { 'm', 'b', 'c', 'a' };
	static size_t const gl_stColorAtlasDataBegin = 128;

	if (memcmp(&sHead->dwMagic, gl_acaString, sizeof(DWORD)))
		return FALSE;
	if (sHead->dwMinVersion > MARBLE_VERSION)                  return FALSE;
	if (sHead->stBeginOfData < gl_stColorAtlasDataBegin)       return FALSE;
	if (!Marble_Color_IsValidColorFormat(sHead->iColorFormat)) return FALSE;

	return TRUE;
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

int Marble_ColorAtlas_GetColorByIndex(Marble_ColorAtlas *sAtlas, size_t stIndex, void *ptrColor) {
	if (sAtlas) {
		if (sAtlas->sColorTable->stSize > stIndex) {
			ptrColor = sAtlas->sColorTable->ptrpData[stIndex];

			return Marble_ErrorCode_Ok;
		}

		ptrColor = NULL;
		return Marble_ErrorCode_ArraySubscript;
	}

	return Marble_ErrorCode_Parameter;
}
#pragma endregion


int Marble_Atlas_Create(int iType, void **ptrpAtlas) {
	int iErrorCode = Marble_ErrorCode_Ok;

	switch (iType) {		
		case Marble_AtlasType_ColorAtlas: 
			Marble_IfError(
				Marble_ColorAtlas_Internal_Create((Marble_ColorAtlas **)ptrpAtlas),
				Marble_ErrorCode_Ok,
				return iErrorCode
			);

			break;
		default: iErrorCode = Marble_ErrorCode_AtlasType;
	}

	return iErrorCode;
}

void Marble_Atlas_Destroy(void **ptrpAtlas) {
	if (ptrpAtlas && *ptrpAtlas) {
		switch (((Marble_Atlas *)*ptrpAtlas)->iAtlasType) {
			case Marble_AtlasType_ColorAtlas: Marble_ColorAtlas_Internal_Destroy((Marble_ColorAtlas **)ptrpAtlas);
		}

		free(*ptrpAtlas);
		*ptrpAtlas = NULL;
	}
}


