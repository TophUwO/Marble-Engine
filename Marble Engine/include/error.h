#pragma once


typedef enum Marble_ErrorCodes {
	Marble_ErrorCode_Ok = 0,
	Marble_ErrorCode_Unknown,

	Marble_ErrorCode_CreateDebugConsole,
	Marble_ErrorCode_MemoryAllocation,
	Marble_ErrorCode_RegisterWindowClass,
	Marble_ErrorCode_CreateWindow,

	__Marble_NumErrorCodes__
} Marble_ErrorCode;


