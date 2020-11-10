#include "win-wsaapi.h"

#define MAX_AUDIO_FRAME_SIZE 192000
#define BUFFER_TIME_100NS (5 * 10000000)

#include <iostream>

namespace RL {
	namespace RecordCapture {

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

			initFormat();

			LogInstance()->rlog(IRecordLog::LOG_INFO, 
				"default audio device speaker SamplerPerSec: %d, nChannel: %d, BitPerSample: %d",
				wfex->nSamplesPerSec,wfex->nChannels,wfex->wBitsPerSample);

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

		void WSAAPISource::initFormat()
		{
			switch (wfex->wFormatTag)
			{
			case WAVE_FORMAT_IEEE_FLOAT: {
				wfex->wFormatTag = WAVE_FORMAT_PCM;
				wfex->wBitsPerSample = 16;
				wfex->nBlockAlign = wfex->wBitsPerSample / 8 * wfex->nChannels;
				wfex->nAvgBytesPerSec = wfex->nSamplesPerSec * wfex->nBlockAlign;
			}break;
			case WAVE_FORMAT_EXTENSIBLE: {
				PWAVEFORMATEXTENSIBLE pEx = (PWAVEFORMATEXTENSIBLE)(wfex.Get());
				if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
					pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
					pEx->Samples.wValidBitsPerSample = 16;
					wfex->wBitsPerSample = 16;
					wfex->nBlockAlign = wfex->wBitsPerSample / 8 * wfex->nChannels;
					wfex->nAvgBytesPerSec = wfex->nSamplesPerSec * wfex->nBlockAlign;
				}
			}break;
			default:break;
			}
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

						frame = framesAvailable * wfex->nChannels * wfex->wBitsPerSample / 8;
						if (dflage & AUDCLNT_BUFFERFLAGS_SILENT)
							memset(buffer, 0, frame);
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

		//microphone
		DUPL_RETURN WSAAPISource::Init(std::shared_ptr<Thread_Data> data, const Microphone& microphone) {
			Data = data;
			AudioDeviceInfo_ = microphone.audioDeviceInfo;
			input_ = true;
			//initialize microphone 
			HRESULT hr;

			ComPtr<IMMDeviceEnumerator> enumerator;
			hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
				CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void **)enumerator.Assign());
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = enumerator->GetDefaultAudioEndpoint(input_ ? eCapture : eRender, eCommunications, device.Assign());
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			device->AddRef();
			hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(client.Assign()));
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			hr = client->GetMixFormat(&wfex);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			initFormat();

			LogInstance()->rlog(IRecordLog::LOG_INFO,
				"default audio device microphone SamplerPerSec: %d, nChannel: %d, BitPerSample: %d",
				wfex->nSamplesPerSec, wfex->nChannels, wfex->wBitsPerSample);

			hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK , BUFFER_TIME_100NS, 0, wfex, nullptr);
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

		DUPL_RETURN WSAAPISource::ProcessFrame(const Microphone& currentMicrophone) {

			//DWORD waitResult = WaitForSingleObject(receiveSignal_, INFINITE);
			HANDLE handle[] = { receiveSignal_,stopSignal_ };
			WaitForMultipleObjects(2, handle, false, 3000);
			UINT32 framesAvailable = 0;
			HRESULT hr = capture->GetNextPacketSize(std::addressof(framesAvailable));
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;
			while (framesAvailable) {
				DWORD dflage = 0;
				hr = capture->GetBuffer(std::addressof(buffer), std::addressof(framesAvailable),
					std::addressof(dflage), nullptr, nullptr);
				if (SUCCEEDED(hr)) {

					if (framesAvailable > 0) {

						frame = framesAvailable * wfex->nChannels * wfex->wBitsPerSample / 8;
						if (dflage & AUDCLNT_BUFFERFLAGS_SILENT)
							memset(buffer, 0, frame);
						if (Data->MicrophoneCaptureData.onAudioFrame) {
							AudioFrame audioFrame;
							audioFrame.buffer = (void*)buffer;
							audioFrame.bytesPerSample = wfex->wBitsPerSample / 8;
							audioFrame.samples = framesAvailable;
							audioFrame.channels = wfex->nChannels;
							audioFrame.samplesPerSec = wfex->nSamplesPerSec;
							audioFrame.renderTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							Data->MicrophoneCaptureData.onAudioFrame(audioFrame);
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
	}//namespace RecordCapture.
}//namespace RL