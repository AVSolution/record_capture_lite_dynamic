#ifndef __WIN_WSAAPI_H__
#define __WIN_WSAAPI_H__

#include "Capture.h"
#include "SCCommon.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

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
			LPBYTE buffer = nullptr;

		public:
			WSAAPISource();
			~WSAAPISource();
			
			void Pause();
			void Resume();
			DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Speaker& speaker);
			DUPL_RETURN ProcessFrame(const Speaker& currentSpeaker);
			DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Microphone& microphone);
			DUPL_RETURN ProcessFrame(const Microphone& currentMicrophone);
		};

	}//namespace RecordCapture.
}//namespace RL
#endif

