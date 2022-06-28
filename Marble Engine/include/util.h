#pragma once

#include <api.h>


typedef struct Marble_Util_Vector {
	void   **ptrpData;
	size_t   stSize;
	size_t   stCapacity;

	int (*onDestroy)(void **ptrObject);
} Marble_Util_Vector;

extern int            Marble_Util_Vector_Create(Marble_Util_Vector **ptrpVector, size_t stStartCapacity, int (*onDestroy)(void **ptrpObject));
extern void           Marble_Util_Vector_Destroy(Marble_Util_Vector **ptrpVector);
extern void   inline  Marble_Util_Vector_SetOnDestroy(Marble_Util_Vector *sVector, int (*onDestroy)(void **ptrpObject));
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


