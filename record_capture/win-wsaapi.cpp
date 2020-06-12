#include "win-wsaapi.h"

#define MAX_AUDIO_FRAME_SIZE 192000
#define BUFFER_TIME_100NS (5 * 10000000)

#include <iostream>

namespace RL {
	namespace Record_Capture {

		WSAAPISource::WSAAPISource()
		{
			CoInitializeEx(NULL, COINIT_MULTITHREADED);
			receiveSignal_ = CreateEvent(nullptr, false, false, nullptr);
			stopSignal_ = CreateEvent(nullptr, true, false, nullptr);
		}

		WSAAPISource::~WSAAPISource()
		{
			SetEvent(stopSignal_);
			CoUninitialize();
		}

		void WSAAPISource::Pause() {

		}
		void WSAAPISource::Resume() {

		}
		
		DUPL_RETURN WSAAPISource::Init(std::shared_ptr<Thread_Data> data, const Speaker& speaker) {
			Data = data;
			AudioDeviceInfo_ = speaker.audioDeviceInfo;
			input_ = false;
			HRESULT hr;

			ComPtr<IMMDeviceEnumerator> enumerator;
			hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
				CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void **)enumerator.Assign());
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = enumerator->GetDefaultAudioEndpoint(input_ ? eCapture : eRender, eMultimedia, device.Assign());
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			device->AddRef();
			hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(client.Assign()));
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = client->GetMixFormat(&wfex);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_LOOPBACK, BUFFER_TIME_100NS, 0, wfex,nullptr);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = client->GetService(__uuidof(IAudioCaptureClient),
				(void **)capture.Assign());
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = client->SetEventHandle(receiveSignal_);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			client->Start();

			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}
		DUPL_RETURN WSAAPISource::ProcessFrame(const Speaker& currentSpeaker) {
			
			//DWORD waitResult = WaitForSingleObject(receiveSignal_, INFINITE);
			HANDLE handle[] = { receiveSignal_,stopSignal_ };
			WaitForMultipleObjects(2, handle, false, 10);
			UINT32 framesAvailable = 0;
			HRESULT hr = capture->GetNextPacketSize(std::addressof(framesAvailable));
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;
			while (framesAvailable) {
				DWORD dflage = 0;
				hr = capture->GetBuffer(std::addressof(buffer), std::addressof(framesAvailable),
					std::addressof(dflage),nullptr,nullptr);
				if (SUCCEEDED(hr)) {
					if (framesAvailable > 0) {
						if (dflage & AUDCLNT_BUFFERFLAGS_SILENT)
							buffer = nullptr;
						frame = framesAvailable * wfex->nChannels * wfex->wBitsPerSample / 8;
						if (Data->SpeakerCaptureData.onAudioFrame) {
							AudioFrame audioFrame;
							audioFrame.buffer = (void*)buffer;
							audioFrame.bytesPerSample = wfex->wBitsPerSample / 8;
							audioFrame.samples = framesAvailable;
							audioFrame.channels = wfex->nChannels;
							audioFrame.samplesPerSec = wfex->nSamplesPerSec;
							audioFrame.renderTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							Data->SpeakerCaptureData.onAudioFrame(audioFrame);
						}
					}
				}
				capture->ReleaseBuffer(framesAvailable);
				hr = capture->GetNextPacketSize(std::addressof(framesAvailable));
				if (FAILED(hr))
					return DUPL_RETURN_ERROR_EXPECTED;
			}
			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}

		DUPL_RETURN WSAAPISource::Init(std::shared_ptr<Thread_Data> data, const Microphone& microphone) {
			Data = data;
			AudioDeviceInfo_ = microphone.audioDeviceInfo;
			input_ = true;
			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}
		DUPL_RETURN WSAAPISource::ProcessFrame(const Microphone& currentMicrophone) {
			AudioDeviceInfo_ = currentMicrophone.audioDeviceInfo;
			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}
	}//namespace Record_Capture.
}//namespace RL