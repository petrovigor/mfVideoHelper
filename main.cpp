#include "autoMFAPI.h"
#include "autoGdipAPI.h"
#include "mfVideoHelper.h"

class Engine {
private:
	AutoMFAPI mfAPI;
	AutoGdiplusAPI gdipAPI;

public:
	HRESULT decode(const std::wstring & file, long iStartSecond, long iSecondsMax) {
		HR_ENTRY;

		mfVideoHelper mfvh(file, iStartSecond, iSecondsMax);

		std::cout << "Frame rate = " << std::to_string(mfvh.GetFrameRate()) << "\n";
		std::cout << "Duration (seconds) = " << std::to_string(mfvh.GetDuration()) << "\n";

		while(mfvh.HaveFrame())
			mfvh.GrabNextFrame(L"c:\\users\\i.petrov\\desktop\\tmp\\frame.bmp");

		HR_RET;
	}
};

void game1() {
	Engine game1;
	game1.decode(L"c:\\users\\i.petrov\\desktop\\echo-hereweare.mp4", 10, 1);
}

int main() {
	game1();
	return 0;
}
