#pragma once

#include <api.h>


typedef struct Marble_Util_Vector {
	void   **ptrpData;
	size_t   stSize;
	size_t   stCapacity;
	size_t   stStartCapacity;

	void (*onDestroy)(void **ptrObject);
} Marble_Util_Vector;

MARBLE_API int            Marble_Util_Vector_Create(Marble_Util_Vector **ptrpVector, size_t stStartCapacity, void (*onDestroy)(void **ptrpObject));
MARBLE_API void           Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector);
MARBLE_API void           Marble_Util_Vector_Clear(Marble_Util_Vector **ptrpVector, _Bool blDoFree);
MARBLE_API void   inline  Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, int (*onDestroy)(void **ptrpObject));
MARBLE_API int    inline  Marble_Util_Vector_PushBack(Marble_Util_Vector *sVector, void *ptrObject);
MARBLE_API int    inline  Marble_Util_Vector_PushFront(Marble_Util_Vector *sVector, void *ptrObject);
MARBLE_API void   inline *Marble_Util_Vector_PopBack(Marble_Util_Vector *sVector, _Bool blDoFree);
MARBLE_API void   inline *Marble_Util_Vector_PopFront(Marble_Util_Vector *sVector, _Bool blDoFree);
MARBLE_API int            Marble_Util_Vector_Insert(Marble_Util_Vector *sVector, size_t stIndex, void *ptrObject);
MARBLE_API void          *Marble_Util_Vector_Erase(Marble_Util_Vector *sVector, size_t stIndex, _Bool blDoFree);
MARBLE_API size_t         Marble_Util_Vector_Find(Marble_Util_Vector *sVector, void *ptrObject, size_t stStartIndex, size_t stEndIndex);


typedef struct Marble_Util_Clock {
	LARGE_INTEGER uStartTime;
	LARGE_INTEGER uStopTime;
} Marble_Util_Clock;

MARBLE_API void   inline Marble_Util_Clock_Start(Marble_Util_Clock *sClock);
MARBLE_API void   inline Marble_Util_Clock_Stop(Marble_Util_Clock *sClock);
MARBLE_API double inline Marble_Util_Clock_AsSeconds(Marble_Util_Clock *sClock);
MARBLE_API double inline Marble_Util_Clock_AsMilliseconds(Marble_Util_Clock *sClock);
MARBLE_API double inline Marble_Util_Clock_AsMicroseconds(Marble_Util_Clock *sClock);
MARBLE_API double inline Marble_Util_Clock_AsNanoseconds(Marble_Util_Clock *sClock);


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

MARBLE_API int         Marble_Util_FileStream_Open(TCHAR const *strPath, int iPermissions, Marble_Util_FileStream **ptrpFileStream);
MARBLE_API void        Marble_Util_FileStream_Destroy(Marble_Util_FileStream **ptrpFileStream);
MARBLE_API void        Marble_Util_FileStream_Close(Marble_Util_FileStream *sFileStream);
MARBLE_API int         Marble_Util_FileStream_Goto(Marble_Util_FileStream *sFileStream, size_t stNewPos);
MARBLE_API int         Marble_Util_FileStream_ReadSize(Marble_Util_FileStream *sFileStream, size_t stSizeInBytes, void *ptrDest);
MARBLE_API int  inline Marble_Util_FileStream_ReadBYTE(Marble_Util_FileStream *sFileStream, uint8_t *bpDest);
MARBLE_API int  inline Marble_Util_FileStream_ReadWORD(Marble_Util_FileStream *sFileStream, uint16_t *wpDest);
MARBLE_API int  inline Marble_Util_FileStream_ReadDWORD(Marble_Util_FileStream *sFileStream, uint32_t *dwpDest);
MARBLE_API int  inline Marble_Util_FileStream_ReadQWORD(Marble_Util_FileStream *sFileStream, uint64_t *qwpDest);


