#include <application.h>


#pragma region color table
typedef struct Marble_ColorTableAsset {
	Marble_Asset;
	struct Marble_Internal_ColorAtlasHead {
		DWORD  dwMagic;
		DWORD  dwMinVersion;
		CHAR   astrIdent[64];
		size_t stNumOfEntries;
		int    iColorFormat;
		size_t stBeginOfData;
	} sHead;
	Marble_Util_Vector *sColorTable;
} Marble_ColorTableAsset;


void Marble_ColorTableAsset_Destroy(Marble_ColorTableAsset *ptrColorTable) {
	if (!ptrColorTable || ptrColorTable->iAssetType ^ Marble_AssetType_ColorTable)
		return;

	Marble_Util_Vector_Destroy(&ptrColorTable->sColorTable);
}


static void Marble_ColorTable_Internal_DestroyColorEntry(void **ptrpColorEntry) {
	free(*ptrpColorEntry);
	*ptrpColorEntry = NULL;
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


int Marble_Asset_CreateColorTableAsset(Marble_Asset **ptrpColorTable) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpColorTable, sizeof(Marble_ColorTableAsset), FALSE, FALSE))
		return iErrorCode;

	(*ptrpColorTable)->iAssetType = Marble_AssetType_ColorTable;
	
	Marble_ColorTableAsset *sColorTableAsset = (Marble_ColorTableAsset *)*ptrpColorTable;
	if (iErrorCode = Marble_Util_Vector_Create(&sColorTableAsset->sColorTable, 32, (void (*)(void **))&Marble_ColorTable_Internal_DestroyColorEntry)) {
		Marble_ColorTableAsset_Destroy(sColorTableAsset);
	
		return iErrorCode;
	}
	memset(&sColorTableAsset->sHead, 0, sizeof(struct Marble_Internal_ColorAtlasHead));
	
	return iErrorCode;
}

int Marble_ColorTableAsset_LoadFromFile(Marble_ColorTableAsset *sColorTable, TCHAR const *strPath) { MARBLE_ERRNO
	Marble_Util_FileStream *sFile = NULL;
	if (iErrorCode = Marble_Util_FileStream_Open(strPath, Marble_Util_StreamPerm_Read, &sFile))
		return iErrorCode;
	
	Marble_IfError(
		iErrorCode = Marble_Util_FileStream_ReadSize(
			sFile, 
			sizeof(struct Marble_Internal_ColorAtlasHead),
			&sColorTable->sHead
		), 
		Marble_ErrorCode_Ok, 
		goto ON_ERROR
	);
	Marble_IfError(
		Marble_ColorAtlas_Internal_ValidateHead(&sColorTable->sHead), 
		TRUE, {
			iErrorCode = Marble_ErrorCode_HeadValidation; 
	
			goto ON_ERROR;
		}
	);
	Marble_IfError(
		iErrorCode = Marble_Util_FileStream_Goto(
			sFile, 
			sColorTable->sHead.stBeginOfData
		), Marble_ErrorCode_Ok, 
		goto ON_ERROR
	);
	
	size_t const stEntrySize = Marble_Color_GetColorSizeByFormat(sColorTable->sHead.iColorFormat);
	for (size_t stIndex = 0; stIndex < sColorTable->sHead.stNumOfEntries; stIndex++) {
		void *ptrColorEntry = NULL;
		if (!(ptrColorEntry = malloc(stEntrySize))) {
			iErrorCode = Marble_ErrorCode_MemoryAllocation;
	
			goto ON_ERROR;
		}
	
		Marble_IfError(
			iErrorCode = Marble_Util_FileStream_ReadSize(sFile, stEntrySize, ptrColorEntry),
			Marble_ErrorCode_Ok, {
				Marble_Util_Vector_Clear(&sColorTable->sColorTable, TRUE);
	
				goto ON_ERROR;
			}
		);
	
		Marble_Util_Vector_PushBack(sColorTable->sColorTable, ptrColorEntry);
	}
	
	ON_ERROR:
	Marble_Util_FileStream_Destroy(&sFile);
	
	return iErrorCode;
}

int Marble_ColorTable_GetColorByIndex(Marble_Asset *sColorTable, size_t stIndex, void *ptrColor) {
	if (sColorTable) {
		Marble_ColorTableAsset *sColorTableAsset = (Marble_ColorTableAsset *)sColorTable;

		if (sColorTableAsset->sColorTable->stSize > stIndex) {
			ptrColor = sColorTableAsset->sColorTable->ptrpData[stIndex];

			return Marble_ErrorCode_Ok;
		}

		ptrColor = NULL;
		return Marble_ErrorCode_ArraySubscript;
	}

	return Marble_ErrorCode_Parameter;
}
#pragma endregion


size_t Marble_Color_GetColorSizeByFormat(int iColorFormat) {
	static struct Marble_Internal_ColorFormatEntry { int iFormat; size_t stSizeInBytes; } const gl_saColorFormatTable[] = {
		{ Marble_ColorFormat_Unknown,   0                 },

		{ Marble_ColorFormat_RGBAfloat, 4 * sizeof(float) },
		{ Marble_ColorFormat_RGBAbyte,  4 * sizeof(BYTE)  }
	};

	if (iColorFormat < 0 || iColorFormat >= __MARBLE_NUMCOLORFORMATS__)
		return Marble_Color_GetColorSizeByFormat(Marble_ColorFormat_Unknown);

	return gl_saColorFormatTable[iColorFormat].stSizeInBytes;
}

_Bool Marble_Color_IsValidColorFormat(int iColorFormat) {
	return iColorFormat > Marble_ColorFormat_Unknown && iColorFormat < __MARBLE_NUMCOLORFORMATS__;
}


