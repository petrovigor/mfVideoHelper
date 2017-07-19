#pragma once

#ifndef __HR_UTILS_H__
#define __HR_UTILS_H__

#include <windows.h>				/* MessageBoxW */
#include <stdio.h>					/* swprintf */

#define HR_ENTRY					HRESULT hr = S_OK
#define HR_RET						return hr

#define	DebugTrap					__debugbreak

#define HR_CHECK(expr)				hr = expr;			\
									if(FAILED(expr)) {	\
										return hr;		\
									}

#define HR_MESSAGE(expr, msg)		hr = expr;																														\
									if(FAILED(hr)) {																												\
										wchar_t _msg[260];																											\
										swprintf(_msg, TEXT("Error in statement:\n\t%s\nFailed with hr = 0x%08x\nDescription:\n\n") TEXT(msg), TEXT(#expr), hr);	\
										MessageBoxW(0, _msg, TEXT("Error"), 0);																						\
										DebugTrap();																												\
										return hr;																													\
									}

#define CHM(expr, msg)				HR_MESSAGE(expr, msg)
#define CH(expr)					HR_MESSAGE(expr, "")

#endif //__HR_UTILS_H__
