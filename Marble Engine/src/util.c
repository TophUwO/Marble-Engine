#include <application.h>


#pragma region Marble_Util_Vector
static size_t const gl_stDefStartCapacity       = 32;
static double const gl_dblDefCapacityMultiplier = 4.0f / 3.0f;

static int Marble_Util_Vector_Internal_Reallocate(Marble_Util_Vector *sVector, size_t stNewCapacity) {
	void *ptrNew = NULL;
	if (ptrNew = realloc(sVector->ptrData, stNewCapacity * sVector->stObjectSize)) {
		sVector->ptrpData   = ptrNew;
		sVector->stCapacity = stNewCapacity;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_MemoryReallocation;
}

static int Marble_Util_Vector_Internal_InitBuffer(Marble_Util_Vector *sVector) {
	return Marble_System_AllocateMemory(
		sVector->iVectorType == Marble_Util_VectorType_VecOfPointers 
			? (void **)&sVector->ptrpData 
			: &sVector->ptrData
		,
		sVector->stObjectSize * sVector->stCapacity,
		TRUE,
		FALSE
	);
}

static void Marble_Util_Vector_Internal_FreeElements(Marble_Util_Vector *sVector) {
	if (!sVector || !sVector->onDestroy)
		return;

	void const *const ptrBase = sVector->iVectorType == Marble_Util_VectorType_VecOfPointers ? (void *)sVector->ptrpData : sVector->ptrData;

	for (size_t stCount = 0; stCount < sVector->stSize; stCount++) {
		void *ptrElem = ((char *)ptrBase) + sVector->stObjectSize * stCount;

		(void)(
			sVector->iVectorType == Marble_Util_VectorType_VecOfPointers
				? ((void (*)(void **))sVector->onDestroy)((void **)ptrElem)
				: ((void (*)(void *))sVector->onDestroy)(ptrElem)
		);
	}
}

static _Bool inline Marble_Util_Vector_Internal_IsValidVectorType(int iVectorType) {
	return iVectorType > Marble_Util_VectorType_Unknown && iVectorType < __MARBLE_NUMVECTORTYPES__;
}

static void inline __Marble_Util_Vector_Internal_DefCopy__(void *ptrDest, void *ptrSrc, size_t stSizeInBytes) {
	memcpy(ptrDest, ptrSrc, stSizeInBytes);
}

static void inline Marble_Util_Vector_Internal_ComputeMemmoveDestAndSrc(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoMoveBackward, char **ptrpBasePtr, void **ptrpDestPtr, void **ptrpSrcPtr) {
	*ptrpBasePtr = sVector->iVectorType == Marble_Util_VectorType_VecOfPointers ? sVector->ptrpData : sVector->ptrData;

	*ptrpSrcPtr  = *ptrpBasePtr + (stIndex + (size_t)blDoMoveBackward) * sVector->stObjectSize;
	*ptrpDestPtr = *ptrpBasePtr + (stIndex + (size_t)!blDoMoveBackward) * sVector->stObjectSize;
}
 

int Marble_Util_Vector_Create(int iVectorType, size_t stObjectSize, size_t stStartCapacity, void *onDestroy, void (*onCopy)(void *, void *, size_t), Marble_Util_Vector **ptrpVector) { MARBLE_ERRNO
	if (!ptrpVector || !Marble_Util_Vector_Internal_IsValidVectorType(iVectorType) || (iVectorType == Marble_Util_VectorType_VecOfObjects && !stObjectSize))
		return Marble_ErrorCode_InternalParameter;

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpVector, 
		sizeof **ptrpVector, 
		FALSE, 
		FALSE
	));
	stStartCapacity = stStartCapacity ? stStartCapacity : gl_stDefStartCapacity;

	(*ptrpVector)->onDestroy       = onDestroy;
	(*ptrpVector)->onCopy          = onCopy ? onCopy : &__Marble_Util_Vector_Internal_DefCopy__;
	(*ptrpVector)->stCapacity      = stStartCapacity;
	(*ptrpVector)->stStartCapacity = stStartCapacity;
	(*ptrpVector)->stSize          = 0;

	(*ptrpVector)->iVectorType  = iVectorType;
	(*ptrpVector)->stObjectSize = stObjectSize && iVectorType ^ Marble_Util_VectorType_VecOfPointers ? stObjectSize : sizeof(void *);
	if (iErrorCode = Marble_Util_Vector_Internal_InitBuffer(*ptrpVector)) {
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

	switch ((*ptrpVector)->iVectorType) {
		case Marble_Util_VectorType_VecOfPointers: free((*ptrpVector)->ptrpData); break;
		case Marble_Util_VectorType_VecOfObjects:  free((*ptrpVector)->ptrData); break;
	}
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
		return;

	memset((*ptrpVector)->ptrpData, 0, (*ptrpVector)->stObjectSize * (*ptrpVector)->stStartCapacity);
}

void Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, void *onDestroy) {
	if (sVector)
		sVector->onDestroy = onDestroy;
}

void Marble_Util_Vector_SetOnCopy(Marble_Util_Vector *sVector, void (*onCopy)(void *, void *, size_t)) {
	if (sVector)
		sVector->onCopy = onCopy;
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
		MB_IFNOK_RET_CODE(Marble_Util_Vector_Internal_Reallocate(
			sVector, 
			sVector->stCapacity + (size_t)(gl_dblDefCapacityMultiplier * sVector->stCapacity)
		));

	char *ptrBase;
	void *ptrDest, *ptrSrc;
	Marble_Util_Vector_Internal_ComputeMemmoveDestAndSrc(sVector, stIndex, FALSE, &ptrBase, &ptrDest, &ptrSrc);
	memmove(
		ptrDest, 
		ptrSrc, 
		(sVector->stSize++ - stIndex) *  sVector->stObjectSize
	);

	switch (sVector->iVectorType) {
		case Marble_Util_VectorType_VecOfPointers:
			sVector->ptrpData[stIndex] = ptrObject;

			break;
		case Marble_Util_VectorType_VecOfObjects:
			sVector->onCopy((char *)ptrBase + stIndex * sVector->stObjectSize, ptrObject, sVector->stObjectSize);

			break;
	}

	return Marble_ErrorCode_Ok;
}

void *Marble_Util_Vector_Erase(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoFree) {
	if (stIndex >= sVector->stSize)
		return (void *)Marble_ErrorCode_ArraySubscript;

	void *ptrRet = sVector->ptrpData[stIndex];
	if (sVector->onDestroy && blDoFree) {
		(void)(
			sVector->iVectorType == Marble_Util_VectorType_VecOfPointers 
				? ((void (*)(void **))sVector->onDestroy)(&ptrRet)
				: ((void (*)(void *))sVector->onDestroy)(ptrRet)
		);
		
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
	char *ptrBase;
	void *ptrDest, *ptrSrc;
	Marble_Util_Vector_Internal_ComputeMemmoveDestAndSrc(sVector, stIndex, TRUE, &ptrBase, &ptrDest, &ptrSrc);
	memmove(
		ptrDest,
		ptrSrc, 
		(sVector->stSize - stIndex) * sVector->stObjectSize
	);
	/*
		* To avoid having the last element twice (due to memmove not actually moving, but copying
		* blocks while supporting overlapping as opposed to memcpy), we just zero it.
	*/
	memset(ptrBase + --sVector->stSize * sVector->stObjectSize, 0, sVector->stObjectSize);

	return ptrRet;
}

size_t Marble_Util_Vector_Find(Marble_Util_Vector *sVector, void *ptrObject, size_t stStartIndex, size_t stEndIndex) {
	stStartIndex = stStartIndex <= 0 ? 0 : stStartIndex;
	stEndIndex   = stEndIndex   <= 0 ? sVector->stSize - 1 : stEndIndex;
	if (!sVector || !sVector->stSize || stStartIndex >= stEndIndex)
		goto ON_NOT_FOUND;

	for (size_t stIndex = stStartIndex; stIndex < stEndIndex; stIndex++)
		if (sVector->ptrpData[stIndex] == ptrObject)
			return stIndex;

ON_NOT_FOUND:
	return (size_t)(-1);
}

void *Marble_Util_Vector_Get(Marble_Util_Vector *sVector, size_t stIndex) {
	if (!sVector || stIndex >= sVector->stSize)
		return NULL;

	return (char *)sVector->ptrData + sVector->stObjectSize * stIndex; // TODO: fix
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
	_Bool const blIsCreateFlag = iPermissions & Marble_Util_StreamPerm_Create;
	_Bool const blIsAppendFlag = iPermissions & Marble_Util_StreamPerm_Append;

	switch (iPermissions & 0xFF) {
		case Marble_Util_StreamPerm_Read:  return blIsAppendFlag ? TEXT("a+") : TEXT("rb");
		case Marble_Util_StreamPerm_Write: return TEXT("wb");
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

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpFileStream, 
		sizeof **ptrpFileStream, 
		FALSE, 
		FALSE
	));

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

	if (fread(ptrDest, 1, stSizeInBytes, sFileStream->flpFilePointer) ^ stSizeInBytes)
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
	/* Source:  https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c */
	/* License: https://mit-license.org/ */

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
static _Bool Marble_Util_HashTable_Internal_Locate(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrElement, size_t *stpBucketIndex, size_t *stpVecIndex, _Bool (*fnFind)(CHAR const *, void *), void **ptrpFoundPtr) {
	/* Compute bucket index (reuse this value later if the bucket does not exist) */
	Marble_Util_Vector *sRefBucket = NULL;
	if (!(sRefBucket = sHashTable->ptrpStorage[*stpBucketIndex = Marble_Util_HashTable_Internal_Hash(sHashTable, astrKey)])) {
		*stpVecIndex = 0;

		return FALSE;
	}

	/*
		* If no pointer has been passed, we are, instead, looking for an
		* element of a specific key.
		* In this case, it is required to pass a "find" function which analyzes
		* the data in the bucket to find if an element satisfies a condition
		* that declares the element "found", e.g. a key match, an ID match, etc.
	*/
	if (!ptrElement) {
		if (!fnFind || !ptrpFoundPtr)
			return FALSE;

		for (size_t stIndex = 0; stIndex < sRefBucket->stSize; stIndex++)
			if (fnFind(astrKey, *ptrpFoundPtr = sRefBucket->ptrpData[stIndex]))
				return TRUE;

		return *ptrpFoundPtr = NULL;
	}

	/*
		* If the bucket exists, we attempt to find it in there. If it could be found, *stpVecIndex*
		* will hold its index and the function will return TRUE. If not, *stpVecIndex* will 
		* be set to (size_t)(-1), and the function will return FALSE.
	*/
	return (_Bool)((*stpVecIndex = Marble_Util_Vector_Find(sHashTable->ptrpStorage[*stpBucketIndex], ptrElement, 0, 0)) ^ (size_t)(-1));
}


int Marble_Util_HashTable_Create(Marble_Util_HashTable **ptrpHashTable, size_t stNumOfBuckets, void (*onDestroy)(void **)) { MARBLE_ERRNO
	if (!ptrpHashTable)
		return Marble_ErrorCode_InternalParameter;

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpHashTable, 
		sizeof **ptrpHashTable, 
		FALSE, 
		FALSE
	));

	(*ptrpHashTable)->onDestroy     = onDestroy;
	(*ptrpHashTable)->stBucketCount = stNumOfBuckets ? stNumOfBuckets : MARBLE_DEFNUMOFBUCKETS;
	MB_IFNOK_DO_BODY(Marble_System_AllocateMemory(
		(void **)&(*ptrpHashTable)->ptrpStorage, 
		sizeof *(*ptrpHashTable)->ptrpStorage * (*ptrpHashTable)->stBucketCount, 
		TRUE, 
		FALSE
	), {
		Marble_Util_HashTable_Destroy(ptrpHashTable);

		return iErrorCode;
	});

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

void Marble_Util_HashTable_SetOnDestroy(Marble_Util_HashTable *sHashTable, void (*onDestroy)(void **)) {
	if (sHashTable)
		sHashTable->onDestroy = onDestroy;
}

int Marble_Util_HashTable_Insert(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blAllowDuplicate) { MARBLE_ERRNO
	size_t stBucketIndex = 0, stVecIndex = 0;

	if (Marble_Util_HashTable_Internal_Locate(sHashTable, astrKey, ptrObject, &stBucketIndex, &stVecIndex, NULL, NULL))
		if (!blAllowDuplicate)
			return Marble_ErrorCode_DuplicatesNotAllowed;

	/* 
		* *stBucketIndex* will always be populated with the hash index, hence we can just use
		* it here without having to verify it. We may still have to create the bucket
		* in case it does not already exist.
	*/
	if (!sHashTable->ptrpStorage[stBucketIndex])
		MB_IFNOK_RET_CODE(Marble_Util_Vector_Create(
			Marble_Util_VectorType_VecOfPointers, 
			0, 
			32, 
			sHashTable->onDestroy,
			NULL, 
			&sHashTable->ptrpStorage[stBucketIndex]
		));

	/*
		* We destroy our vector in case we fail to insert our element into it; this is because
		* we want to always revert what we did if any sub-action of our action fails. This particular
		* step, however, is debatable, as we might still have to recreate this vector as soon another 
		* element gets hashed into this position again; might as well just leave the vector valid.
	*/
	MB_IFNOK_DO_BODY(Marble_Util_Vector_PushBack(sHashTable->ptrpStorage[stBucketIndex], ptrObject),
		Marble_Util_Vector_Destroy(&sHashTable->ptrpStorage[stBucketIndex]);
	);

	return iErrorCode;
}

void *Marble_Util_HashTable_Erase(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, _Bool (*fnFind)(CHAR const *, void *), _Bool blDoFree) {
	size_t stBucketIndex = 0, stVecIndex = 0;
	
	void *ptrFoundPtr = NULL;
	if (Marble_Util_HashTable_Internal_Locate(sHashTable, astrKey, NULL, &stBucketIndex, &stVecIndex, fnFind, &ptrFoundPtr)) {
		void *ptrElem = Marble_Util_Vector_Erase(sHashTable->ptrpStorage[stBucketIndex], stVecIndex, blDoFree);

		return ptrElem;
	}

	return NULL;
}

void *Marble_Util_HashTable_Find(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, _Bool (*fnFind)(CHAR const *, void *)) {
	if (!sHashTable || !astrKey || !*astrKey || !fnFind)
		return FALSE;

	void *ptrRet = NULL;
	size_t stBucketIndex, stVecIndex;
	Marble_Util_HashTable_Internal_Locate(sHashTable, astrKey, NULL, &stBucketIndex, &stVecIndex, fnFind, &ptrRet);

	return ptrRet;
}
#pragma endregion


#pragma region Marble_Util_Array
int Marble_Util_Array2D_Create(size_t stElementSize, size_t stWidth, size_t stHeight, Marble_Util_Array2D **ptrpArray) { MARBLE_ERRNO
	if (!ptrpArray || !stElementSize || !stWidth || !stHeight)
		return Marble_ErrorCode_InternalParameter;

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpArray, 
		sizeof **ptrpArray,
		FALSE, 
		FALSE
	));

	(*ptrpArray)->stWidth    = stWidth;
	(*ptrpArray)->stHeight   = stHeight;
	(*ptrpArray)->stElemSize = stElementSize;

	MB_IFNOK_DO_BODY(Marble_System_AllocateMemory(
		&(*ptrpArray)->ptrData, 
		stWidth * stHeight * stElementSize, 
		TRUE, 
		FALSE
	), Marble_Util_Array2D_Destroy(ptrpArray));

	return iErrorCode;
}

void Marble_Util_Array2D_Destroy(Marble_Util_Array2D **ptrpArray) {
	if (!ptrpArray || !*ptrpArray)
		return;

	free((*ptrpArray)->ptrData);
	free(*ptrpArray);
	*ptrpArray = NULL;
}

void *Marble_Util_Array2D_Get(Marble_Util_Array2D *sArray, size_t staDimIndices[2]) {
	if (!sArray || staDimIndices[0] >= sArray->stWidth || staDimIndices[1] >= sArray->stHeight)
		return NULL;

	return (void *)((char *)sArray->ptrData + (staDimIndices[0] * sArray->stHeight + staDimIndices[1]));
}
#pragma endregion


