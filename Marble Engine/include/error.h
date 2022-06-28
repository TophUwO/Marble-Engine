#pragma once


typedef enum Marble_ErrorCodes {
	Marble_ErrorCode_Ok = 0,
	Marble_ErrorCode_Unknown,

	Marble_ErrorCode_CreateDebugConsole,
	Marble_ErrorCode_MemoryAllocation,
	Marble_ErrorCode_MemoryReallocation,
	Marble_ErrorCode_RegisterWindowClass,
	Marble_ErrorCode_CreateWindow,
	Marble_ErrorCode_CreateLayer,
	Marble_ErrorCode_InitHighPrecClock,
	Marble_ErrorCode_ArraySubscript
} Marble_ErrorCode;


