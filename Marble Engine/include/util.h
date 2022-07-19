#pragma once

#include <api.h>


typedef struct Marble_Util_Vector {
	void   **ptrpData;
	size_t   stSize;
	size_t   stCapacity;
	size_t   stStartCapacity;

	void (*onDestroy)(void **ptrObject);
} Marble_Util_Vector;

int            Marble_Util_Vector_Create(Marble_Util_Vector **ptrpVector, size_t stStartCapacity, void (*onDestroy)(void **ptrpObject));
void           Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector);
void           Marble_Util_Vector_Clear(Marble_Util_Vector **ptrpVector, _Bool blDoFree, _Bool blDoDownsize);
void   inline  Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, void (*onDestroy)(void **ptrpObject));
int    inline  Marble_Util_Vector_PushBack(Marble_Util_Vector *sVector, void *ptrObject);
int    inline  Marble_Util_Vector_PushFront(Marble_Util_Vector *sVector, void *ptrObject);
void   inline *Marble_Util_Vector_PopBack(Marble_Util_Vector *sVector, _Bool blDoFree);
void   inline *Marble_Util_Vector_PopFront(Marble_Util_Vector *sVector, _Bool blDoFree);
int            Marble_Util_Vector_Insert(Marble_Util_Vector *sVector, size_t stIndex, void *ptrObject);
void          *Marble_Util_Vector_Erase(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoFree);
size_t         Marble_Util_Vector_Find(Marble_Util_Vector *sVector, void *ptrObject, size_t stStartIndex, size_t stEndIndex);


typedef struct Marble_Util_Clock {
	LARGE_INTEGER uStartTime;
	LARGE_INTEGER uStopTime;
} Marble_Util_Clock;

void   inline Marble_Util_Clock_Start(Marble_Util_Clock *sClock);
void   inline Marble_Util_Clock_Stop(Marble_Util_Clock *sClock);
double inline Marble_Util_Clock_AsSeconds(Marble_Util_Clock *sClock);
double inline Marble_Util_Clock_AsMilliseconds(Marble_Util_Clock *sClock);
double inline Marble_Util_Clock_AsMicroseconds(Marble_Util_Clock *sClock);
double inline Marble_Util_Clock_AsNanoseconds(Marble_Util_Clock *sClock);


enum Marble_Util_StreamPerms {
	Marble_Util_StreamPerm_Read   = 1 << 0,
	Marble_Util_StreamPerm_Write  = 1 << 1,
	Marble_Util_StreamPerm_Append = 1 << 8,
	Marble_Util_StreamPerm_Create = 1 << 9
};

typedef struct Marble_Util_FileStream {
	FILE        *flpFilePointer;
	struct stat  sInfo;
	int          iPermissions;
} Marble_Util_FileStream;

int  Marble_Util_FileStream_Open(TCHAR const *strPath, int iPermissions, Marble_Util_FileStream **ptrpFileStream);
void Marble_Util_FileStream_Destroy(Marble_Util_FileStream **ptrpFileStream);
void Marble_Util_FileStream_Close(Marble_Util_FileStream *sFileStream);
int  Marble_Util_FileStream_Goto(Marble_Util_FileStream *sFileStream, size_t stNewPos);
int  Marble_Util_FileStream_ReadSize(Marble_Util_FileStream *sFileStream, size_t stSizeInBytes, void *ptrDest);
int  Marble_Util_FileStream_ReadBYTE(Marble_Util_FileStream *sFileStream, uint8_t *bpDest);
int  Marble_Util_FileStream_ReadWORD(Marble_Util_FileStream *sFileStream, uint16_t *wpDest);
int  Marble_Util_FileStream_ReadDWORD(Marble_Util_FileStream *sFileStream, uint32_t *dwpDest);
int  Marble_Util_FileStream_ReadQWORD(Marble_Util_FileStream *sFileStream, uint64_t *qwpDest);


typedef struct Marble_Util_HashTable {
	size_t stBucketCount;
	Marble_Util_Vector **ptrpStorage;

	void (*onDestroy)(void **ptrpObject);
} Marble_Util_HashTable;

int         Marble_Util_HashTable_Create(Marble_Util_HashTable **ptrpHashTable, size_t stNumOfBuckets, void (*onDestroy)(void **ptrpObject));
void        Marble_Util_HashTable_Destroy(Marble_Util_HashTable **ptrpHashTable);
void inline Marble_Util_HashTable_SetOnDestroy(Marble_Util_HashTable *sHashTable, void (*onDestroy)(void **ptrpObject));
int         Marble_Util_HashTable_Insert(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blAllowDuplicate);
int         Marble_Util_HashTable_Erase(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blDoFree);


