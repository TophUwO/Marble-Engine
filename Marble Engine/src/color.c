#include <application.h>


#pragma region color table
typedef struct Marble_ColorTableAsset {
	Marble_Asset;

	struct Marble_Internal_ColorTableHead {
		DWORD dwNumOfEntries;
		int   iColorFormat;
	} sHead;
	Marble_Util_Vector *sColorTable;
} Marble_ColorTableAsset;


void Marble_ColorTableAsset_Destroy(Marble_ColorTableAsset *ptrColorTable) {
	if (!ptrColorTable || (ptrColorTable->iAssetType & 0xFF) ^ Marble_AssetType_ColorTable)
		return;

	Marble_Util_Vector_Destroy(&ptrColorTable->sColorTable);
}


static void Marble_ColorTable_Internal_DestroyColorEntry(void **ptrpColorEntry) {
	free(*ptrpColorEntry);
	*ptrpColorEntry = NULL;
}

static _Bool Marble_ColorAtlas_Internal_ValidateHead(Marble_AssetHead *sAssetHead) {
	static CHAR const gl_acaMagicStr[sizeof(DWORD)] = { 'm', 'b', Marble_AssetType_ColorTable, 0 };

	if (memcmp(&sAssetHead->dwMagic, gl_acaMagicStr, sizeof(DWORD)))
		return FALSE;

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
	memset(&sColorTableAsset->sHead, 0, sizeof(sColorTableAsset->sHead));
	
	return iErrorCode;
}

int Marble_ColorTableAsset_LoadFromFile(Marble_ColorTableAsset *sColorTable, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead) { MARBLE_ERRNO
	Marble_IfError(
		iErrorCode = Marble_Util_FileStream_ReadSize(
			sStream, 
			sizeof(sColorTable->sHead),
			&sColorTable->sHead
		), 
		Marble_ErrorCode_Ok, 
		goto ON_ERROR
	);
	Marble_IfError(
		Marble_ColorAtlas_Internal_ValidateHead(sAssetHead), 
		TRUE, {
			iErrorCode = Marble_ErrorCode_HeadValidation; 
	
			goto ON_ERROR;
		}
	);
	
	size_t const stEntrySize = Marble_Color_GetColorSizeByFormat(sColorTable->sHead.iColorFormat);
	for (DWORD dwIndex = 0; dwIndex < sColorTable->sHead.dwNumOfEntries; dwIndex++) {
		void *ptrColorEntry = NULL;
		if (iErrorCode = Marble_System_AllocateMemory(&ptrColorEntry, stEntrySize, FALSE, FALSE))
			goto ON_ERROR;
	
		Marble_IfError(
			iErrorCode = Marble_Util_FileStream_ReadSize(sStream, stEntrySize, ptrColorEntry),
			Marble_ErrorCode_Ok, {
				Marble_Util_Vector_Clear(&sColorTable->sColorTable, TRUE);
	
				goto ON_ERROR;
			}
		);
	
		Marble_Util_Vector_PushBack(sColorTable->sColorTable, ptrColorEntry);
	}
	
ON_ERROR:
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


