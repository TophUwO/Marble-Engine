#include <application.h>


#pragma region Marble_Util_Vector
static size_t const gl_stDefStartCapacity       = 32;
static double const gl_dblDefCapacityMultiplier = 4.0f / 3.0f;

static int Marble_Util_Vector_Internal_Reallocate(Marble_Util_Vector *sVector) {
	size_t const stNewCap = sVector->stCapacity + (size_t)(gl_dblDefCapacityMultiplier * sVector->stCapacity);

	void *ptrNew = NULL;
	if (ptrNew = realloc(sVector->ptrpData, stNewCap * sizeof(*sVector->ptrpData))) {
		sVector->ptrpData   = ptrNew;
		sVector->stCapacity = stNewCap;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_MemoryReallocation;
}


int Marble_Util_Vector_Create(Marble_Util_Vector **ptrpVector, size_t stStartCapacity, int (*onDestroy)(void **ptrpObject)) {
	if (*ptrpVector = malloc(sizeof(**ptrpVector))) {
		stStartCapacity = stStartCapacity ? stStartCapacity : gl_stDefStartCapacity;

		(*ptrpVector)->onDestroy  = onDestroy;
		(*ptrpVector)->stCapacity = stStartCapacity;
		(*ptrpVector)->stSize     = 0;

		if (!((*ptrpVector)->ptrpData = calloc(1, sizeof(*(*ptrpVector)->ptrpData) * (*ptrpVector)->stCapacity))) {
			free(*ptrpVector);
			*ptrpVector = NULL;

			goto ON_ERROR;
		}

		return Marble_ErrorCode_Ok;
	}

ON_ERROR:
	return Marble_ErrorCode_MemoryAllocation;
}

void Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector) {
	if ((*ptrpVector)->onDestroy)
		for (size_t stCount = 0; stCount < (*ptrpVector)->stSize; stCount++)
			(*ptrpVector)->onDestroy(&(*ptrpVector)->ptrpData[stCount]);

	free((*ptrpVector)->ptrpData);
	free(*ptrpVector);
	*ptrpVector = NULL;
}

void Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, int (*onDestroy)(void **ptrpObject)) {
	sVector->onDestroy = onDestroy;
}

int Marble_Util_Vector_PushBack(Marble_Util_Vector *sVector, void *ptrObject) {
	return Marble_Util_Vector_Insert(sVector, sVector->stSize, ptrObject);
}

int Marble_Util_Vector_PushFront(Marble_Util_Vector *sVector, void *ptrObject) {
	return Marble_Util_Vector_Insert(sVector, 0, ptrObject);
}

void *Marble_Util_Vector_PopBack(Marble_Util_Vector *sVector, _Bool blDoFree) {
	return Marble_Util_Vector_Erase(sVector, sVector->stSize - 1, blDoFree);
}

void *Marble_Util_Vector_PopFront(Marble_Util_Vector *sVector, _Bool blDoFree) {
	return Marble_Util_Vector_Erase(sVector, 0, blDoFree);
}

int Marble_Util_Vector_Insert(Marble_Util_Vector *sVector, size_t stIndex, void *ptrObject) {
	if (sVector->stSize >= sVector->stCapacity) {
		int iErrorCode = Marble_ErrorCode_Ok;

		if (iErrorCode = Marble_Util_Vector_Internal_Reallocate(sVector))
			return iErrorCode;
	}

	memmove(
		&sVector->ptrpData[stIndex + 1], 
		&sVector->ptrpData[stIndex], 
		(sVector->stSize++ - stIndex) * sizeof(*sVector->ptrpData)
	);
	sVector->ptrpData[stIndex] = ptrObject;

	return Marble_ErrorCode_Ok;
}

void *Marble_Util_Vector_Erase(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoFree) {
	if (stIndex >= sVector->stSize)
		return (void *)Marble_ErrorCode_ArraySubscript;

	if (sVector->onDestroy && blDoFree)
		sVector->onDestroy(&sVector->ptrpData[stIndex]);

	void *ptrRet = sVector->ptrpData[stIndex];
	memmove(
		&sVector->ptrpData[stIndex],
		&sVector->ptrpData[stIndex + 1], 
		(sVector->stSize - stIndex) * sizeof(*sVector->ptrpData)
	);
	sVector->ptrpData[--sVector->stSize] = NULL;

	return ptrRet ? ptrRet : (void *)Marble_ErrorCode_Ok;
}

size_t Marble_Util_Vector_Find(Marble_Util_Vector *sVector, void *ptrObject, size_t stStartIndex, size_t stEndIndex) {
	stStartIndex = stStartIndex <= 0 ? 0 : stStartIndex;
	stEndIndex   = stEndIndex   <= 0 ? sVector->stSize - 1 : stEndIndex;
	if (!sVector->stSize || stStartIndex >= stEndIndex)
		goto ON_NOT_FOUND;

	for (size_t stIndex = stStartIndex; stIndex < stEndIndex; stIndex++)
		if (sVector->ptrpData[stIndex] == ptrObject)
			return stIndex;

ON_NOT_FOUND:
	return (size_t)(-1);
}
#pragma endregion


#pragma region Marble_Util_Clock
static void inline Marble_Util_Clock_Internal_Reset(Marble_Util_Clock *sClock) {
	memset(sClock, 0, sizeof(*sClock));
}


void Marble_Util_Clock_Start(Marble_Util_Clock *sClock) {
	Marble_Util_Clock_Internal_Reset(sClock);

	QueryPerformanceCounter(&sClock->uStartTime);
}

void Marble_Util_Clock_Stop(Marble_Util_Clock *sClock) {
	QueryPerformanceCounter(&sClock->uStopTime);
}

double Marble_Util_Clock_AsSeconds(Marble_Util_Clock *sClock) {
	return (double)(sClock->uStopTime.QuadPart - sClock->uStartTime.QuadPart) / (double)gl_sApplication.uPerfFreq.QuadPart;
}

double Marble_Util_Clock_AsMilliseconds(Marble_Util_Clock *sClock) {
	return (double)(sClock->uStopTime.QuadPart - sClock->uStartTime.QuadPart) / ((double)gl_sApplication.uPerfFreq.QuadPart / 1e+3);
}

double Marble_Util_Clock_AsMicroseconds(Marble_Util_Clock *sClock) {
	return (double)(sClock->uStopTime.QuadPart - sClock->uStartTime.QuadPart) / ((double)gl_sApplication.uPerfFreq.QuadPart / 1e+6);
}

double Marble_Util_Clock_AsNanoseconds(Marble_Util_Clock *sClock) {
	return (double)(sClock->uStopTime.QuadPart - sClock->uStartTime.QuadPart) / ((double)gl_sApplication.uPerfFreq.QuadPart / 1e+9);
}
#pragma endregion


#pragma region Marble_Util_FileStream
static TCHAR inline const *const Marble_Util_Stream_Internal_GetStringFromPermissions(int iPermissions) {
	_Bool blIsCreateFlag = iPermissions & Marble_Util_StreamPerm_Create;
	_Bool blIsAppendFlag = iPermissions & Marble_Util_StreamPerm_Append;

	switch (iPermissions & 0xFF) {
		case Marble_Util_StreamPerm_Read:  return blIsAppendFlag ? TEXT("a+") : TEXT("r");
		case Marble_Util_StreamPerm_Write: return TEXT("w");
		case Marble_Util_StreamPerm_Read | Marble_Util_StreamPerm_Write: 
			return blIsCreateFlag ? TEXT("w+") : TEXT("r+");
		case 0:
			if (blIsAppendFlag)
				return TEXT("a");
	}

	return TEXT("");
}


int Marble_Util_FileStream_Open(TCHAR const *strPath, int iPermissions, Marble_Util_FileStream **ptrpFileStream) {
	if (strPath && ptrpFileStream) {
		if (!(*ptrpFileStream = malloc(sizeof(**ptrpFileStream))))
			return Marble_ErrorCode_MemoryAllocation;

		_tfopen_s(&(*ptrpFileStream)->flpFilePointer, strPath, Marble_Util_Stream_Internal_GetStringFromPermissions((*ptrpFileStream)->iPermissions = iPermissions));
		if (!(*ptrpFileStream)->flpFilePointer)
			return Marble_ErrorCode_OpenFile;

		if (fstat(_fileno((*ptrpFileStream)->flpFilePointer), &(*ptrpFileStream)->sInfo))
			return Marble_ErrorCode_GetFileInfo;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}

void Marble_Util_FileStream_Destroy(Marble_Util_FileStream **ptrpFileStream) {
	if (ptrpFileStream && *ptrpFileStream) {
		Marble_Util_FileStream_Close(*ptrpFileStream);

		free(*ptrpFileStream);
		*ptrpFileStream = NULL;
	}
}

void Marble_Util_FileStream_Close(Marble_Util_FileStream *sFileStream) {
	if (sFileStream)
		fclose(sFileStream->flpFilePointer);
}

int Marble_Util_FileStream_ReadSize(Marble_Util_FileStream *sFileStream, size_t stSizeInBytes, void *ptrDest) {
	if (!sFileStream || !ptrDest || !stSizeInBytes)
		return Marble_ErrorCode_Parameter;

	if (fread(ptrDest, stSizeInBytes, 1, sFileStream->flpFilePointer) ^ stSizeInBytes)
		return Marble_ErrorCode_ReadFromFile;

	return Marble_ErrorCode_Ok;
}

int Marble_Util_FileStream_ReadBYTE(Marble_Util_FileStream *sFileStream, uint8_t *bpDest) {
	return Marble_Util_FileStream_ReadSize(sFileStream, sizeof(uint8_t), bpDest);
}

int Marble_Util_FileStream_ReadWORD(Marble_Util_FileStream *sFileStream, uint16_t *wpDest) {
	return Marble_Util_FileStream_ReadSize(sFileStream, sizeof(uint16_t), wpDest);
}

int Marble_Util_FileStream_ReadDWORD(Marble_Util_FileStream *sFileStream, uint32_t *dwpDest) {
	return Marble_Util_FileStream_ReadSize(sFileStream, sizeof(uint32_t), dwpDest);
}

int Marble_Util_FileStream_ReadQWORD(Marble_Util_FileStream *sFileStream, uint64_t *qwpDest) {
	return Marble_Util_FileStream_ReadSize(sFileStream, sizeof(uint64_t), qwpDest);
}
#pragma endregion


