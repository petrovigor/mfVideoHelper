#pragma once

#ifndef __MF_VIDEO_HELPER_H__
#define __MF_VIDEO_HELPER_H__

#include "hr_utils.h"

#include <iostream>
#include <memory>
#include <string>
#include <atlbase.h>

/* mf api */
#include <mfapi.h>
#include <mfidl.h>			/* must be before "mfreadwrite.h" */
#include <mfreadwrite.h>

/* gdi+ api */
#include <gdiplus.h>

class mfVideoHelper {
private:
	CComPtr<IMFSourceReader> sourceReader;
	uint32_t iFrameOffset;
	uint32_t iFramesRequired;
	uint32_t iFramesFetched;
	uint32_t iFps;
	uint32_t dur;
	bool bNoMoreFrames;

public:
	mfVideoHelper(const std::wstring &file, long iSecondsOffset, long iFramesMax) : iFrameOffset(0), iFramesRequired(0), iFramesFetched(0), iFps(0), dur(0), bNoMoreFrames(false) {
		Init(file, iSecondsOffset, iFramesMax);
	}

	~mfVideoHelper() { }

	bool HaveFrame() const {return !bNoMoreFrames;} /* have more frames to extract */
	uint32_t GetDuration() const {return dur;}
	uint32_t GetFrameRate() const {return iFps;}

	HRESULT GrabNextFrame(const std::wstring &outFile, bool bSkip = false) {
		HR_ENTRY;

		if(bNoMoreFrames)
			return S_OK;

		DWORD dwFlags;
		CComPtr<IMFSample> pSample;
		CH(sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, 0, &dwFlags, 0, &pSample.p));

		if(!bSkip) {
			UINT32 nWidth, nHeight;
			IMFMediaType *videoType = NULL;
			CH(sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &videoType));
			CH(MFGetAttributeSize(videoType, MF_MT_FRAME_SIZE, &nWidth, &nHeight));

			CComPtr<IMFMediaBuffer> buffer;
			BYTE* data;
			CH(pSample->ConvertToContiguousBuffer(&buffer));

			DWORD Size;
			CH(buffer->Lock(&data, 0, &Size));

			std::unique_ptr<Gdiplus::Bitmap> bitmap = std::unique_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(nWidth, nHeight, nWidth * 4, PixelFormat32bppRGB, data));
			const CLSID pngEncoderClsId = { 0x557cf406, 0x1a04, 0x11d3,{ 0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e } };
			//const std::wstring image = L"c:\\users\\i.petrov\\desktop\\tmp\\" + std::to_wstring(iFramesFetched) + L"output.bmp";
			bitmap->Save(outFile.c_str(), &pngEncoderClsId);

			CH(buffer->Unlock());

			++iFramesFetched;
			if(iFramesFetched >= iFramesRequired)
				bNoMoreFrames = true;
		}

		HR_RET;
	}

private:
	HRESULT Init(const std::wstring &file, long iSecondsOffset, long iRequiredMax) {
		HR_ENTRY;

		CComPtr<IMFAttributes> att;
		CH(MFCreateAttributes(&att, 1));
		CH(att->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE));

		CH(MFCreateSourceReaderFromURL(file.c_str(), att.p, &sourceReader.p));

		CComPtr<IMFMediaType> output_type;
		CH(MFCreateMediaType(&output_type));
		CH(output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		CH(output_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32));
		
		CH(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, output_type));

		CH(getDurationInSeconds(&dur));
		CH(getFrameRate(&iFps));

		iFramesRequired = iRequiredMax * iFps;
		iFrameOffset = iSecondsOffset * iFps;
		iFramesFetched = 0;

		if(iSecondsOffset > 0)
			for(int i = 0; i < iFrameOffset; ++i)
				GrabNextFrame(L"", true);

		HR_RET;
	}

	HRESULT getFrameRate(UINT32 *fps) {
		HR_ENTRY;

		UINT32 dummy;
		CComPtr<IMFMediaType> videoType;
		CH(sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &videoType.p));
		CH(MFGetAttributeSize(videoType, MF_MT_FRAME_RATE, fps, &dummy));

		HR_RET;
	}

	HRESULT getDurationInSeconds(uint32_t *dur) {
		HR_ENTRY;
        PROPVARIANT prop;
        CH(sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop));
        *dur = (prop.uhVal.QuadPart / 10000000);
		HR_RET;
	}
};

#endif //__MF_VIDEO_HELPER_H__
