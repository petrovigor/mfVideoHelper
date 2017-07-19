#pragma once

#ifndef __AUTO_MF_API_H__
#define __AUTO_MF_API_H__

#include "hr_utils.h"
#include <mfapi.h>

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

class AutoMFAPI {
public:
	AutoMFAPI() {
		Init();
	}

	~AutoMFAPI() {
		Shutdown();
	}

private:
	HRESULT Init() {
		HR_ENTRY;
		CH(CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));
		CH(MFStartup(MF_VERSION));
		HR_RET;
	}

	HRESULT Shutdown() {
		HR_ENTRY;
		CH(MFShutdown());
		CoUninitialize();
		HR_RET;
	}
};

#endif //__AUTO_MF_API_H__
