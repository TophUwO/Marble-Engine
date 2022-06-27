#include <util.h>
#include <error.h>


static size_t const gl_stDefStartCapacity     = 32;
static float const  gl_fDefCapacityMultiplier = 4.0f / 3.0f;

static int Marble_Util_Vector_Internal_Reallocate(Marble_Util_Vector *sVector) {
	size_t const stNewCap = sVector->stCapacity + (size_t)(gl_fDefCapacityMultiplier * sVector->stCapacity);

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


