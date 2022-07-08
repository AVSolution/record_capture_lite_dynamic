#ifndef __WIN_WSAAPI_H__
#define __WIN_WSAAPI_H__

#include "Capture.h"
#include "SCCommon.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

#include "../speex_resampler/include/speex/speex_resampler.h"
#ifdef _WIN64
#pragma comment(lib,"../speex_resampler/lib/x64/libspeexdsp")
#else
#pragma comment(lib,"../speex_resampler/lib/x86/libspeexdsp")
#endif

namespace RL {
	namespace RecordCapture {

		class WSAAPISource : public BaseFrameProcessor {
			ComPtr<IMMDevice> device;
			ComPtr<IAudioClient> client;
			ComPtr<IAudioCaptureClient> capture;
			ComPtr<IAudioRenderClient> render;

			AudioDeviceInfo AudioDeviceInfo_;
			bool input_;
			HANDLE receiveSignal_;
			HANDLE stopSignal_;
			CoTaskMemPtr<WAVEFORMATEX> wfex;
			UINT32 frame = 0;
			/*LPBYTE*/ uint8_t* buffer = nullptr;
			SpeexResamplerState *resampler = NULL;
			/*LPBYTE*/uint8_t* bufferOut = nullptr;

		public:
			WSAAPISource();
			~WSAAPISource();
			
			void Pause();
			void Resume();
			void initFormat(bool input = true);
			void initSpeexSrc(bool input = true);
			void uninitSpeexSrc();
			DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Speaker& speaker);
			DUPL_RETURN ProcessFrame(const Speaker& currentSpeaker);
			DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Microphone& microphone);
			DUPL_RETURN ProcessFrame(const Microphone& currentMicrophone);
		};

	}//namespace RecordCapture.
}//namespace RL
#endif

