#include <application.h>


#pragma region color table
typedef struct Marble_ColorTableAsset {
	Marble_Asset;

	struct Marble_ColorTableAsset_Head {
		DWORD dwNumOfEntries;
		int   iColorFormat;
	} sHead;
	Marble_Util_Vector *sColorTable;
} Marble_ColorTableAsset;


int Marble_Asset_CreateColorTableAsset(Marble_Asset **ptrpColorTable) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpColorTable, sizeof(Marble_ColorTableAsset), TRUE, FALSE))
		return iErrorCode;
	
	(*ptrpColorTable)->iAssetType = Marble_AssetType_ColorTable;
	
	return Marble_ErrorCode_Ok;
}

void Marble_ColorTableAsset_Destroy(Marble_ColorTableAsset *ptrColorTable) {
	if (!ptrColorTable || (ptrColorTable->iAssetType & 0xFF) ^ Marble_AssetType_ColorTable)
		return;

	Marble_Util_Vector_Destroy(&ptrColorTable->sColorTable);
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
	
	size_t stEntrySize;
	if (iErrorCode = Marble_ColorTable_InitializeContainer(sColorTable, sColorTable->sHead.iColorFormat, &stEntrySize))
		return iErrorCode;

	for (DWORD dwIndex = 0; dwIndex < sColorTable->sHead.dwNumOfEntries; dwIndex++) {
		BYTE baBuffer[64] = { 0 };

		if (iErrorCode = Marble_Util_FileStream_ReadSize(sStream, stEntrySize, baBuffer))
			return iErrorCode;
	
		Marble_Util_Vector_PushBack(sColorTable->sColorTable, baBuffer);
	}
	
ON_ERROR:
	return iErrorCode;
}

int Marble_ColorTable_GetColorByIndex(Marble_ColorTableAsset *sColorTable, size_t stIndex, void *ptrColor) {
	if (sColorTable) {
		if (sColorTable->sColorTable->stSize > stIndex) {
			ptrColor = sColorTable->sColorTable->ptrpData[stIndex];

			return Marble_ErrorCode_Ok;
		}

		ptrColor = NULL;
		return Marble_ErrorCode_ArraySubscript;
	}

	return Marble_ErrorCode_Parameter;
}

int Marble_ColorTable_InitializeContainer(Marble_ColorTableAsset *sColorTable, int iColorFormat, size_t *stpColorEntrySize) { MARBLE_ERRNO
	size_t const stElemSize = Marble_Color_GetColorSizeByFormat(iColorFormat);
	if (!stElemSize)
		return Marble_ErrorCode_Parameter;

	return Marble_Util_Vector_Create(Marble_Util_VectorType_VecOfObjects, *stpColorEntrySize = stElemSize, 32, NULL, NULL, &sColorTable->sColorTable);
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


