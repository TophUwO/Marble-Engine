#pragma once

#include <api.h>


typedef enum Marble_Util_VectorTypes {
	Marble_Util_VectorType_Unknown = 0,

	Marble_Util_VectorType_VecOfPointers,
	Marble_Util_VectorType_VecOfObjects,

	__MARBLE_NUMVECTORTYPES__
} Marble_Util_VectorType;

typedef struct Marble_Util_Vector {
	size_t stSize;
	size_t stCapacity;
	size_t stStartCapacity;
	size_t stObjectSize;

	int iVectorType;
	union {
		void  *ptrData;
		void **ptrpData;
	};

	void *onDestroy;
	void  (*onCopy)(void *ptrDest, void *ptrSrc, size_t stSizeInBytes);
} Marble_Util_Vector;

extern int            Marble_Util_Vector_Create(int iVectorType, size_t stObjectSize, size_t stStartCapacity, void *onDestroy, void (*onCopy)(void *ptrDest, void *ptrSrc, size_t stSizeInBytes), Marble_Util_Vector **ptrpVector);
extern void           Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector);
extern void           Marble_Util_Vector_Clear(Marble_Util_Vector **ptrpVector, _Bool blDoFree, _Bool blDoDownsize);
extern void   inline  Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, void *onDestroy);
extern void   inline  Marble_Util_Vector_SetOnCopy(Marble_Util_Vector *sVector, void (*onCopy)(void *ptrDest, void *ptrSrc, size_t stSizeInBytes));
extern int    inline  Marble_Util_Vector_PushBack(Marble_Util_Vector *sVector, void *ptrObject);
extern int    inline  Marble_Util_Vector_PushFront(Marble_Util_Vector *sVector, void *ptrObject);
extern void   inline *Marble_Util_Vector_PopBack(Marble_Util_Vector *sVector, _Bool blDoFree);
extern void   inline *Marble_Util_Vector_PopFront(Marble_Util_Vector *sVector, _Bool blDoFree);
extern int            Marble_Util_Vector_Insert(Marble_Util_Vector *sVector, size_t stIndex, void *ptrObject);
extern void          *Marble_Util_Vector_Erase(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoFree);
extern size_t         Marble_Util_Vector_Find(Marble_Util_Vector *sVector, void *ptrObject, size_t stStartIndex, size_t stEndIndex);


typedef struct Marble_Util_Clock {
	LARGE_INTEGER uStartTime;
	LARGE_INTEGER uStopTime;
} Marble_Util_Clock;

extern void   inline Marble_Util_Clock_Start(Marble_Util_Clock *sClock);
extern void   inline Marble_Util_Clock_Stop(Marble_Util_Clock *sClock);
extern double inline Marble_Util_Clock_AsSeconds(Marble_Util_Clock *sClock);
extern double inline Marble_Util_Clock_AsMilliseconds(Marble_Util_Clock *sClock);
extern double inline Marble_Util_Clock_AsMicroseconds(Marble_Util_Clock *sClock);
extern double inline Marble_Util_Clock_AsNanoseconds(Marble_Util_Clock *sClock);


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

extern int         Marble_Util_FileStream_Open(TCHAR const *strPath, int iPermissions, Marble_Util_FileStream **ptrpFileStream);
extern void        Marble_Util_FileStream_Destroy(Marble_Util_FileStream **ptrpFileStream);
extern void        Marble_Util_FileStream_Close(Marble_Util_FileStream *sFileStream);
extern int         Marble_Util_FileStream_Goto(Marble_Util_FileStream *sFileStream, size_t stNewPos);
extern int  inline Marble_Util_FileStream_ReadSize(Marble_Util_FileStream *sFileStream, size_t stSizeInBytes, void *ptrDest);
extern int  inline Marble_Util_FileStream_ReadBYTE(Marble_Util_FileStream *sFileStream, uint8_t *bpDest);
extern int  inline Marble_Util_FileStream_ReadWORD(Marble_Util_FileStream *sFileStream, uint16_t *wpDest);
extern int  inline Marble_Util_FileStream_ReadDWORD(Marble_Util_FileStream *sFileStream, uint32_t *dwpDest);
extern int  inline Marble_Util_FileStream_ReadQWORD(Marble_Util_FileStream *sFileStream, uint64_t *qwpDest);


typedef struct Marble_Util_HashTable {
	size_t stBucketCount;
	Marble_Util_Vector **ptrpStorage;

	void (*onDestroy)(void **ptrpObject);
} Marble_Util_HashTable;

extern int         Marble_Util_HashTable_Create(Marble_Util_HashTable **ptrpHashTable, size_t stNumOfBuckets, void (*onDestroy)(void **ptrpObject));
extern void        Marble_Util_HashTable_Destroy(Marble_Util_HashTable **ptrpHashTable);
extern void inline Marble_Util_HashTable_SetOnDestroy(Marble_Util_HashTable *sHashTable, void (*onDestroy)(void **ptrpObject));
extern int         Marble_Util_HashTable_Insert(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blAllowDuplicate);
extern int         Marble_Util_HashTable_Erase(Marble_Util_HashTable *sHashTable, CHAR const *astrKey, void *ptrObject, _Bool blDoFree);


typedef struct Marble_Util_Array2D {
	size_t  stWidth;
	size_t  stHeight;
	size_t  stElemSize;

	void **ptrpData;
} Marble_Util_Array2D;

extern int   Marble_Util_Array2D_Create(size_t stElementSize, size_t stWidth, size_t stHeight, Marble_Util_Array2D **ptrpArray);
extern void  Marble_Util_Array2D_Destroy(Marble_Util_Array2D **ptrpArray);
extern void *Marble_Util_Array2D_Get(Marble_Util_Array2D *sArray, size_t staDimIndices[2]);


