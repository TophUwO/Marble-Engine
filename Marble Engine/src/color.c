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


int Marble_ColorTableAsset_Create(void const *ptrCreateParams, Marble_ColorTableAsset **ptrpColorTable) { MARBLE_ERRNO
	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpColorTable, 
		sizeof(Marble_ColorTableAsset), 
		TRUE, 
		FALSE
	));
	(*ptrpColorTable)->iAssetType = Marble_AssetType_ColorTable;
	
	return Marble_ErrorCode_Ok;
}

void Marble_ColorTableAsset_Destroy(Marble_ColorTableAsset *ptrColorTable) {
	if (!ptrColorTable || (ptrColorTable->iAssetType & 0xFF) ^ Marble_AssetType_ColorTable)
		return;

	Marble_Util_Vector_Destroy(&ptrColorTable->sColorTable);
}

int Marble_ColorTableAsset_LoadFromFile(Marble_ColorTableAsset *sColorTable, Marble_Util_FileStream *sStream, Marble_CommonAssetHead *sAssetHead) { MARBLE_ERRNO
	MB_IFNOK_RET_CODE(Marble_Util_FileStream_ReadSize(
		sStream, 
		sizeof sColorTable->sHead,
		&sColorTable->sHead
	));
	
	size_t stEntrySize;
	MB_IFNOK_RET_CODE(Marble_ColorTable_InitializeContainer(
		sColorTable, 
		sColorTable->sHead.iColorFormat, 
		&stEntrySize
	));

	for (DWORD dwIndex = 0; dwIndex < sColorTable->sHead.dwNumOfEntries; dwIndex++) {
		BYTE baBuffer[64] = { 0 };
		MB_IFNOK_RET_CODE(Marble_Util_FileStream_ReadSize(
			sStream, 
			stEntrySize, 
			baBuffer
		));
	
		Marble_Util_Vector_PushBack(sColorTable->sColorTable, baBuffer);
	}
	
	return iErrorCode;
}

int Marble_ColorTable_GetColorByIndex(Marble_ColorTableAsset *sColorTable, size_t stIndex, void *ptrColor) {
	if (!sColorTable)
		return Marble_ErrorCode_Parameter;

	if (sColorTable->sColorTable->stSize > stIndex) {
		ptrColor = sColorTable->sColorTable->ptrpData[stIndex];

		return Marble_ErrorCode_Ok;
	}

	*(DWORD *)ptrColor = 0;
	return Marble_ErrorCode_ArraySubscript;
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


