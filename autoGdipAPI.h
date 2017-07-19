#pragma once

#ifndef __AUTO_GDIP_API_H__
#define __AUTO_GDIP_API_H__

#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

class AutoGdiplusAPI {
private:
	Gdiplus::GdiplusStartupInput startInput;
	ULONG_PTR token;

public:
	AutoGdiplusAPI() {
		Init();
	}

	~AutoGdiplusAPI() {
		Shutdown();
	}
private:
	void Init() {
		Gdiplus::GdiplusStartup(&token, &startInput, 0);
	}

	void Shutdown() {
		Gdiplus::GdiplusShutdown(token);
	}
};

#endif //__AUTO_GDIP_API_H__
