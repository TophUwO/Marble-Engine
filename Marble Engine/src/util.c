#include <application.h>


#pragma region Marble_Util_Vector
static size_t const gl_stDefStartCapacity       = 32;
static double const gl_dblDefCapacityMultiplier = 4.0f / 3.0f;

static int Marble_Util_Vector_Internal_Reallocate(Marble_Util_Vector *sVector, size_t stNewCapacity) {
	void *ptrNew = NULL;
	if (ptrNew = realloc(sVector->ptrpData, stNewCapacity * sizeof(*sVector->ptrpData))) {
		sVector->ptrpData   = ptrNew;
		sVector->stCapacity = stNewCapacity;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_MemoryReallocation;
}

static int Marble_Util_Vector_Internal_InitBuffer(void **ptrpBuffer, size_t stCapacity) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpBuffer, sizeof(*ptrpBuffer) * stCapacity, TRUE, FALSE))
		return iErrorCode;

	return Marble_ErrorCode_Ok;
}

static void Marble_Util_Vector_Internal_FreeElements(Marble_Util_Vector *sVector) {
	if (sVector->onDestroy)
		for (size_t stCount = 0; stCount < sVector->stSize; stCount++)
			sVector->onDestroy(&sVector->ptrpData[stCount]);
}


int Marble_Util_Vector_Create(Marble_Util_Vector **ptrpVector, size_t stStartCapacity, void (*onDestroy)(void **ptrpObject)) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpVector, sizeof(**ptrpVector), FALSE, FALSE))
		return iErrorCode;

	stStartCapacity = stStartCapacity ? stStartCapacity : gl_stDefStartCapacity;

	(*ptrpVector)->onDestroy       = onDestroy;
	(*ptrpVector)->stCapacity      = stStartCapacity;
	(*ptrpVector)->stStartCapacity = stStartCapacity;
	(*ptrpVector)->stSize          = 0;

	if (iErrorCode = Marble_Util_Vector_Internal_InitBuffer((void **)&(*ptrpVector)->ptrpData, (*ptrpVector)->stCapacity)) {
		free(*ptrpVector);
		*ptrpVector = NULL;

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

void Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector) {
	if (!ptrpVector || !*ptrpVector)
		return;

	Marble_Util_Vector_Internal_FreeElements(*ptrpVector);

	free((*ptrpVector)->ptrpData);
	free(*ptrpVector);
	*ptrpVector = NULL;
}

void Marble_Util_Vector_Clear(Marble_Util_Vector **ptrpVector, _Bool blDoFree, _Bool blDoDownsize) {
	if (!ptrpVector || !*ptrpVector)
		return;

	if (blDoFree)
		Marble_Util_Vector_Internal_FreeElements(*ptrpVector);
	(*ptrpVector)->stSize = 0;

	if (blDoDownsize && Marble_Util_Vector_Internal_Reallocate(*ptrpVector, (*ptrpVector)->stStartCapacity))
		Marble_Util_Vector_Destroy(ptrpVector);

	memset((*ptrpVector)->ptrpData, 0, sizeof(*(*ptrpVector)->ptrpData) * (*ptrpVector)->stStartCapacity);
}

void Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, void (*onDestroy)(void **ptrpObject)) {
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

int Marble_Util_Vector_Insert(Marble_Util_Vector *sVector, size_t stIndex, void *ptrObject) { MARBLE_ERRNO
	if (sVector->stSize >= sVector->stCapacity)
		if (iErrorCode = Marble_Util_Vector_Internal_Reallocate(sVector, sVector->stCapacity + (size_t)(gl_dblDefCapacityMultiplier * sVector->stCapacity)))
			return iErrorCode;

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

	void *ptrRet = sVector->ptrpData[stIndex];
	if (sVector->onDestroy && blDoFree) {
		sVector->onDestroy(&ptrRet);
		
		/* 
			* It's very likely that onDestroy will already zero *ptrRet* 
			* (all internal clean-up functions used for this container do);
			* However, this API may be exposed to the user at some point, hence
			* we have no control over what the function will do to the pointer itself,
			* so we just zero it just to make sure.
		*/
		ptrRet = NULL;
	}

	/*
		* Overwrite deleted (thus, now invalid) index with the memory block that follows it until
		* the end by moving the block by -1 indices.
	*/
	memmove(
		&sVector->ptrpData[stIndex],
		&sVector->ptrpData[stIndex + 1], 
		(sVector->stSize - stIndex) * sizeof(*sVector->ptrpData)
	);
	/*
		* To avoid having the last element twice (due to memmove not actually moving, but copying
		* blocks while supporting overlapping as opposed to memcpy), we just zero it.
	*/
	sVector->ptrpData[--sVector->stSize] = NULL;

	return ptrRet;
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


int Marble_Util_FileStream_Open(TCHAR const *strPath, int iPermissions, Marble_Util_FileStream **ptrpFileStream) { MARBLE_ERRNO
	if (!strPath || !*strPath || !ptrpFileStream)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_System_AllocateMemory(ptrpFileStream, sizeof(**ptrpFileStream), FALSE, FALSE))
		return iErrorCode;

	_tfopen_s(&(*ptrpFileStream)->flpFilePointer, strPath, Marble_Util_Stream_Internal_GetStringFromPermissions((*ptrpFileStream)->iPermissions = iPermissions));

	if (!(*ptrpFileStream)->flpFilePointer) {
		Marble_Util_FileStream_Destroy(ptrpFileStream);

		return Marble_ErrorCode_OpenFile;
	}

	if (fstat(_fileno((*ptrpFileStream)->flpFilePointer), &(*ptrpFileStream)->sInfo)) {
		Marble_Util_FileStream_Destroy(ptrpFileStream);

		return Marble_ErrorCode_GetFileInfo;
	}

	return Marble_ErrorCode_Ok;
}

void Marble_Util_FileStream_Destroy(Marble_Util_FileStream **ptrpFileStream) {
	if (!ptrpFileStream || !*ptrpFileStream)
		return;

	Marble_Util_FileStream_Close(*ptrpFileStream);

	free(*ptrpFileStream);
	*ptrpFileStream = NULL;
}

void Marble_Util_FileStream_Close(Marble_Util_FileStream *sFileStream) {
	if (sFileStream && sFileStream->flpFilePointer)
		fclose(sFileStream->flpFilePointer);
}

int Marble_Util_FileStream_Goto(Marble_Util_FileStream *sFileStream, size_t stNewPos) {
	if (!sFileStream)
		return Marble_ErrorCode_InternalParameter;

	if (_fseeki64(sFileStream->flpFilePointer, stNewPos, SEEK_SET))
		return Marble_ErrorCode_GotoFilePosition;

	return Marble_ErrorCode_Ok;
}

int Marble_Util_FileStream_ReadSize(Marble_Util_FileStream *sFileStream, size_t stSizeInBytes, void *ptrDest) {
	if (!sFileStream || !ptrDest || !stSizeInBytes)
		return Marble_ErrorCode_Parameter;

	if (fread(ptrDest, stSizeInBytes, 1, sFileStream->flpFilePointer) ^ 1)
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


#pragma region Marble_Util_HashTable
#define MARBLE_DEFNUMOFBUCKETS (64)


static uint32_t inline Marble_Util_HashTable_Internal_MurmurHash(CHAR const *astrKey) {
	/* https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c */

	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;
	uint32_t r1 = 15;
	uint32_t r2 = 13;
	uint32_t m  = 5;
	uint32_t n  = 0xe6546b64;
	uint32_t h  = 0;
	uint32_t k  = 0;
	uint8_t *d  = (uint8_t *)astrKey;

	const uint32_t *chunks = NULL;
	const uint8_t  *tail   = NULL;
	int i = 0;
	int const iLen = (int)strlen(astrKey);
	int l = iLen / 4;
	
	h = gl_sApplication.ui32HashSeed;

	chunks = (const uint32_t *)(d + (size_t)l * 4);
	tail   = (const uint8_t *)(d + (size_t)l * 4);

	for (i = -l; i != 0; ++i) {
		k = chunks[i];

		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;

		h ^= k;
		h = (h << r2) | (h >> (32 - r2));
		h = h * m + n;
	}
	k = 0;

	switch (iLen & 3) {
		case 3: k ^= (tail[2] << 16);
		case 2: k ^= (tail[1] << 8);
		case 1:
			k ^= tail[0];
			k *= c1;
			k = (k << r1) | (k >> (32 - r1));
			k *= c2;
			h ^= k;
	}
	h ^= iLen;

	h ^= (h >> 16);
	h *= 0x85ebca6b;
	h ^= (h >> 13);
	h *= 0xc2b2ae35;
	h ^= (h >> 16);

	return h;
}

static size_t inline Marble_Util_HashTable_Internal_Hash(Marble_Util_HashTable *sHashTable, CHAR const *astrKey) {
	return (size_t)Marble_Util_HashTable_Internal_MurmurHash(astrKey) % sHashTable->stBucketCount;
}

/*
	* Attempt to locate element; if the element can be located, the function will 
	* return non-zero, both populating *stBucketIndex* and *stVecIndex*. If the element
	* cannot be found, the function will return zero and *stVecIndex* will be 0.
*/
static _Bool Marble_Util_HashTable_Internal_Locate(Marble_Util_HashTable *sHashTable, CHAR const *strKey, void *ptrElement, size_t *stpBucketIndex, size_t *stpVecIndex) {
	/* Compute bucket index (reuse this value later if the bucket does not exist) */
	if (!sHashTable->ptrpStorage[*stpBucketIndex = Marble_Util_HashTable_Internal_Hash(sHashTable, strKey)]) {
		*stpVecIndex = 0;

		return FALSE;
	}

	/*
		* If the bucket exists, we attempt to find it in there. If it could be found, *stpVecIndex*
		* will hold its index and the function will return TRUE. If not, *stpVecIndex* will 
		* be set to (size_t)(-1), and the function will return FALSE.
	*/
	return (_Bool)((*stpVecIndex = Marble_Util_Vector_Find(sHashTable->ptrpStorage[*stpBucketIndex], ptrElement, 0, 0)) ^ (size_t)(-1));
}


int Marble_Util_HashTable_Create(Marble_Util_HashTable **ptrpHashTable, size_t stNumOfBuckets, void (*onDestroy)(void **ptrpObject)) { MARBLE_ERRNO
	if (!ptrpHashTable)
		return Marble_ErrorCode_InternalParameter;

	if (iErrorCode = Marble_System_AllocateMemory(ptrpHashTable, sizeof(**ptrpHashTable), FALSE, FALSE))
		return iErrorCode;

	(*ptrpHashTable)->onDestroy     = onDestroy;
	(*ptrpHashTable)->stBucketCount = stNumOfBuckets ? stNumOfBuckets : MARBLE_DEFNUMOFBUCKETS;
	if (iErrorCode = Marble_System_AllocateMemory((void **)&(*ptrpHashTable)->ptrpStorage, sizeof(*(*ptrpHashTable)->ptrpStorage) * (*ptrpHashTable)->stBucketCount, TRUE, FALSE)) {
		Marble_Util_HashTable_Destroy(ptrpHashTable);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

void Marble_Util_HashTable_Destroy(Marble_Util_HashTable **ptrpHashTable) {
	if (!ptrpHashTable || !*ptrpHashTable)
		return;

	/* Free-up bucket resources */
	if ((*ptrpHashTable)->ptrpStorage)
		for (size_t stIndex = 0; stIndex < (*ptrpHashTable)->stBucketCount; stIndex++)
			Marble_Util_Vector_Destroy(&(*ptrpHashTable)->ptrpStorage[stIndex]);

	/* Free hash table memory itself */
	free((*ptrpHashTable)->ptrpStorage);
	free(*ptrpHashTable);
	*ptrpHashTable = NULL;
}

void Marble_Util_HashTable_SetOnDestroy(Marble_Util_HashTable *sHashTable, void (*onDestroy)(void **ptrpObject)) {
	if (sHashTable)
		sHashTable->onDestroy = onDestroy;
}

int Marble_Util_HashTable_Insert(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blAllowDuplicate) { MARBLE_ERRNO
	size_t stBucketIndex = 0, stVecIndex = 0;

	if (Marble_Util_HashTable_Internal_Locate(sHashTable, astrKey, ptrObject, &stBucketIndex, &stVecIndex))
		if (!blAllowDuplicate)
			return Marble_ErrorCode_DuplicatesNotAllowed;

	/* 
		* *stBucketIndex* will always be populated with the hash index, hence we can just use
		* it here without having to verify it. We may still have to create the bucket
		* in case it does not already exist.
	*/
	if (!sHashTable->ptrpStorage[stBucketIndex])
		if (iErrorCode = Marble_Util_Vector_Create(&sHashTable->ptrpStorage[stBucketIndex], 32, sHashTable->onDestroy))
			return iErrorCode;

	return Marble_Util_Vector_PushBack(sHashTable->ptrpStorage[stBucketIndex], ptrObject);
}

int Marble_Util_HashTable_Erase(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blDoFree) {
	size_t stBucketIndex = 0, stVecIndex = 0;
	
	if (Marble_Util_HashTable_Internal_Locate(sHashTable, astrKey, ptrObject, &stBucketIndex, &stVecIndex)) {
		void *ptrElem = Marble_Util_Vector_Erase(sHashTable->ptrpStorage[stBucketIndex], stVecIndex, blDoFree);

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_ElementNotFound;
}
#pragma endregion


