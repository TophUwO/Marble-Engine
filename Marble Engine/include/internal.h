#pragma once

#include <util.h>
#include <event.h>
#include <layer.h>


#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }


typedef struct Marble_Util_Vector {
	void   **ptrpData;
	size_t   stSize;
	size_t   stCapacity;
	size_t   stStartCapacity;

	void (*onDestroy)(void **ptrObject);
} Marble_Util_Vector;

typedef struct Marble_Util_Clock {
	LARGE_INTEGER uStartTime;
	LARGE_INTEGER uStopTime;
} Marble_Util_Clock;

typedef struct Marble_Util_FileStream {
	FILE        *flpFilePointer;
	struct stat  sInfo;
	int          iPermissions;
} Marble_Util_FileStream;


typedef struct Marble_AssetManager {
	Marble_Util_Vector *sAtlases;
} Marble_AssetManager;

typedef struct Marble_Asset {
	ULONGLONG uqwGlobalAssetId;
	int       iAssetType;
} Marble_Asset;

typedef struct Marble_Atlas {
	Marble_Asset;
	int iAtlasType;
} Marble_Atlas;

typedef struct Marble_ColorAtlas {
	Marble_Atlas;
	Marble_Util_Vector *sColorTable;
} Marble_ColorAtlas;

typedef struct Marble_Layer {
	DWORD  dwLayerId;
	_Bool  blIsEnabled;
	void  *ptrUserdata;

	struct Marble_Layer_Callbacks {
		int (*onPush)(Marble_Layer *sSelf);
		int (*onPop)(Marble_Layer *sSelf);
		int (*onUpdate)(Marble_Layer *sSelf);
		int (*onEvent)(Marble_Layer *sSelf, Marble_Event *sEvent);
	} sCallbacks;
} Marble_Layer;


extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);


