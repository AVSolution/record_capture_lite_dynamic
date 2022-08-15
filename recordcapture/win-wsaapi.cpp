#include "win-wsaapi.h"

#define MAX_AUDIO_FRAME_SIZE 192000
#define BUFFER_TIME_100NS (5 * 10000000)

#include <iostream>

namespace RL {
	namespace RecordCapture {

#define RECORD_AUDIO_RESAMPLE_SAMPLERATE  44100
#define RECORD_AUDIO_RESAMPLE_CHANNEL 2
#define RECORD_AUDIO_RESAMPLE_BYTEPERSAMPLE 2
//#define RECORD_AUDIO_SIMULATION_MIC

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

			uninitSpeexSrc();
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

			initFormat(false);

			CoTaskMemPtr<WCHAR> w_id;
			device->GetId(&w_id);
			char* pszOutput = nullptr; int outputlen = 0;
			_WTA(w_id, wcslen(w_id), std::addressof(pszOutput), std::addressof(outputlen));

			LogInstance()->rlog(IRecordLog::LOG_INFO,
				"default audio device speaker id: %s \n\t SamplerPerSec: %d, nChannel: %d, BitPerSample: %d",
				pszOutput, wfex->nSamplesPerSec, wfex->nChannels, wfex->wBitsPerSample);
			delete[] pszOutput;

			hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_LOOPBACK, BUFFER_TIME_100NS, 0, wfex, nullptr);
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

			initSpeexSrc(false);

			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}

		void WSAAPISource::initFormat(bool input /*= true*/)
		{
			switch (wfex->wFormatTag)
			{
			case WAVE_FORMAT_IEEE_FLOAT: {
				LogInstance()->rlog(IRecordLog::LOG_INFO, "initFormat:  %s WAVE_FORMAT_IEEE_FLOAT", input ? "Microphone Audio" : "Speaker Audio");
				wfex->wFormatTag = WAVE_FORMAT_PCM;
				wfex->wBitsPerSample = 16;
				wfex->nBlockAlign = wfex->wBitsPerSample / 8 * wfex->nChannels;
				wfex->nAvgBytesPerSec = wfex->nSamplesPerSec * wfex->nBlockAlign;
			}break;
			case WAVE_FORMAT_EXTENSIBLE: {
				LogInstance()->rlog(IRecordLog::LOG_INFO, "initFormat: %s WAVE_FORMAT_EXTENSIBLE", input ? "Microphone Audio" : "Speaker Audio");
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

		void WSAAPISource::initSpeexSrc(bool input /*= true*/)
		{
			if (resampler == nullptr) {
				int sampleIn = wfex->nSamplesPerSec;
				int sampleout = RECORD_AUDIO_RESAMPLE_SAMPLERATE;
				int nChannel = wfex->nChannels;
				int err = 0;
				if (sampleIn == sampleout)
					return;
				resampler = speex_resampler_init(nChannel, sampleIn, sampleout, 10, &err);
				speex_resampler_set_rate(resampler, sampleIn, sampleout);
				speex_resampler_skip_zeros(resampler);
				bufferOut = new unsigned char[48000 * 2 * 2];
				memset(bufferOut, 0, 48000 * 2 * 2);
				LogInstance()->rlog(IRecordLog::LOG_INFO, "%s speex src samplerate: %d", input ? "Microphone Audio" : "Speaker Audio", RECORD_AUDIO_RESAMPLE_SAMPLERATE);
			}
		}

		void WSAAPISource::uninitSpeexSrc()
		{
			bufferOut = nullptr;
			if (NULL != resampler) {
				speex_resampler_destroy(resampler);
				delete[] bufferOut;
				resampler = nullptr;
				bufferOut = nullptr;
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
					std::addressof(dflage), nullptr, nullptr);
				if (SUCCEEDED(hr)) {
					if (framesAvailable > 0) {
						frame = framesAvailable * wfex->nChannels * wfex->wBitsPerSample / 8;
						if (dflage & AUDCLNT_BUFFERFLAGS_SILENT)
							memset(buffer, 0, frame);
						if (NULL != resampler) {//active resampler module.
							int ret = 0;
							spx_uint32_t in_len = frame / sizeof(int16_t);
							spx_uint32_t out_len = frame / sizeof(int16_t) *  (1.0 * RECORD_AUDIO_RESAMPLE_SAMPLERATE / wfex->nSamplesPerSec);
							//static FILE* infd = fopen("speed_samplerate_in.pcm", "wb+");
							//fwrite(buffer, sizeof(int16_t), in_len, infd);
							if (wfex->nChannels == 1)
								ret = speex_resampler_process_interleaved_int(resampler, (int16_t*)buffer, &in_len, (int16_t*)bufferOut, &out_len);
							else
								ret = speex_resampler_process_int(resampler, 0, (int16_t*)buffer, &in_len, (int16_t*)bufferOut, &out_len);
							if (ret == RESAMPLER_ERR_SUCCESS) {
								//static FILE* outfd = fopen("speed_samplerate_out.pcm", "wb+");
								//fwrite(bufferOut, sizeof(int16_t), out_len, outfd);
								//printf("processed in_len: [%d]; out_len = %d \n", in_len, out_len);
							}
							else {
								LogInstance()->rlog(IRecordLog::LOG_INFO, "!!!!!speak speex src error: %d",ret);
							}
						}
						else {
							bufferOut = buffer;
						}
						if (Data->SpeakerCaptureData.onAudioFrame) {
							AudioFrame audioFrame;
							audioFrame.buffer = (void*)bufferOut;//(void*)buffer;
							audioFrame.bytesPerSample = wfex->wBitsPerSample / 8;
							audioFrame.samples = framesAvailable * (1.0 * RECORD_AUDIO_RESAMPLE_SAMPLERATE / wfex->nSamplesPerSec);
							audioFrame.channels = wfex->nChannels;
							audioFrame.samplesPerSec = RECORD_AUDIO_RESAMPLE_SAMPLERATE; //wfex->nSamplesPerSec;
							audioFrame.renderTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							if (audioFrame.buffer && 
								Data && Data->SpeakerCaptureData.onAudioFrame)
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

			//enumerator->GetDevice() //get  special deviceId.
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

			initFormat(true);

			CoTaskMemPtr<WCHAR> w_id;
			device->GetId(&w_id);
			char* pszOutput = nullptr; int outputlen = 0;
			_WTA(w_id, wcslen(w_id), std::addressof(pszOutput), std::addressof(outputlen));

			LogInstance()->rlog(IRecordLog::LOG_INFO,
				"default audio device microphone id: %s \n\t SamplerPerSec: %d, nChannel: %d, BitPerSample: %d",
				pszOutput, wfex->nSamplesPerSec, wfex->nChannels, wfex->wBitsPerSample);
			delete[] pszOutput;

			hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, BUFFER_TIME_100NS, 0, wfex, nullptr);
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

			initSpeexSrc(true);

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
#ifdef RECORD_AUDIO_SIMULATION_MIC
						 {//simulation microphone input audio buffer
							frame = 441 * 2 * 2;
							static FILE* fSimulationFile = fSimulationFile = fopen("test_44100_2.pcm", "rb+");
							static std::unique_ptr<uint8_t[]> simuBuffer = std::make_unique<uint8_t[]>(frame);
							if (feof(fSimulationFile))
								fseek(fSimulationFile, 0, SEEK_SET);
							if (fread(simuBuffer.get(), frame, 1, fSimulationFile) != 0)
								buffer = simuBuffer.get();
							else {
								fseek(fSimulationFile, 0, SEEK_SET);
								return DUPL_RETURN_SUCCESS;
							}
						}
#endif
						if (NULL != resampler) {//active resampler module.
							int ret = 0;
							spx_uint32_t in_len = frame / sizeof(int16_t);
							spx_uint32_t out_len = frame / sizeof(int16_t) *  (1.0 * RECORD_AUDIO_RESAMPLE_SAMPLERATE / wfex->nSamplesPerSec);
							if (wfex->nChannels == 1)
								ret = speex_resampler_process_interleaved_int(resampler, (int16_t*)buffer, &in_len, (int16_t*)bufferOut, &out_len);
							else
								ret = speex_resampler_process_int(resampler, 0, (int16_t*)buffer, &in_len, (int16_t*)bufferOut, &out_len);
							if (ret == RESAMPLER_ERR_SUCCESS) {
								//static FILE* outfd = fopen("speed_samplerate.pcm", "wb+");
								//fwrite(bufferOut, sizeof(int16_t), out_len, outfd);
								//printf("processed in_len: [%d]; out_len = %d \n", in_len, out_len);
							}
							else {
								LogInstance()->rlog(IRecordLog::LOG_INFO, "!!!!!microphone speex src error: %d", ret);
							}
						}
						else {
							bufferOut = buffer;
						}
						if (Data->MicrophoneCaptureData.onAudioFrame) {
							AudioFrame audioFrame;
							audioFrame.buffer = (void*)bufferOut;//(void*)buffer;
							audioFrame.bytesPerSample = wfex->wBitsPerSample / 8;
							audioFrame.samples = framesAvailable * (1.0 * RECORD_AUDIO_RESAMPLE_SAMPLERATE / wfex->nSamplesPerSec);
							audioFrame.channels = wfex->nChannels;
							audioFrame.samplesPerSec = RECORD_AUDIO_RESAMPLE_SAMPLERATE; //wfex->nSamplesPerSec;
							audioFrame.renderTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							if (audioFrame.buffer && Data &&  Data->MicrophoneCaptureData.onAudioFrame)
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