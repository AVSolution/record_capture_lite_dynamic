#include "SCCommon.h"
#include "Capture.h"
#include <combaseapi.h>
#pragma comment(lib,"Ole32")
#include <windows.h>
#include <mmdeviceapi.h>
#include <winerror.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <algorithm>

#include <iostream>

namespace RL {
	namespace RecordCapture {

		char* _WTA(__in wchar_t* pszInBuf, __in int nInSize, __out char** pszOutBuf, __out int* pnOutSize)
		{
			if (!pszInBuf || !pszOutBuf || !pnOutSize || nInSize <= 0)return NULL;
			*pnOutSize = WideCharToMultiByte(NULL, NULL, pszInBuf, nInSize, *pszOutBuf, 0, NULL, NULL);
			if (*pnOutSize == 0)return NULL;
			(*pnOutSize)++;
			*pszOutBuf = new char[*pnOutSize];
			memset((void*)*pszOutBuf, 0, sizeof(char)* (*pnOutSize));
			if (WideCharToMultiByte(NULL, NULL, pszInBuf, nInSize, *pszOutBuf, *pnOutSize, NULL, NULL) == 0)
				return NULL;
			else
				return *pszOutBuf;
		}

		std::vector<AudioDeviceInfo> GetAudioDevice(bool input)
		{
			std::vector<AudioDeviceInfo> audioDevices;

			HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (FAILED(hr))
				return audioDevices;

			ComPtr<IMMDeviceEnumerator> enumerator;
			ComPtr<IMMDeviceCollection> collection;
			UINT count;

			hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
				CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
				(void **)enumerator.Assign());
			if (FAILED(hr))
				return audioDevices;

			hr = enumerator->EnumAudioEndpoints(input?eCapture:eRender, DEVICE_STATE_ACTIVE, collection.Assign());
			if (FAILED(hr))
				return audioDevices;

			hr = collection->GetCount(&count);
			for (UINT i = 0; i < count; i++) {
				ComPtr<IMMDevice> device;
				CoTaskMemPtr<WCHAR> w_id;
				AudioDeviceInfo info;

				hr = collection->Item(i, device.Assign());
				if (FAILED(hr))
					continue;

				hr = device->GetId(&w_id);
				if (FAILED(hr) || !w_id || !*w_id)
					continue;

				char* pszOutput = nullptr; int outputlen = 0;
				_WTA(w_id, wcslen(w_id), std::addressof(pszOutput), std::addressof(outputlen));
				strcpy_s(info.id, outputlen, pszOutput);

				ComPtr<IPropertyStore> store;
				HRESULT res;

				if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, store.Assign()))) {
					PROPVARIANT nameVar;

					PropVariantInit(&nameVar);
					res = store->GetValue(PKEY_Device_FriendlyName, &nameVar);

					if (SUCCEEDED(res) && nameVar.pwszVal && *nameVar.pwszVal) {
						size_t len = wcslen(nameVar.pwszVal);
						char* pszOutput = nullptr; int outputlen = 0;
						_WTA(nameVar.pwszVal, len, std::addressof(pszOutput), std::addressof(outputlen));
						strcpy_s(info.name, outputlen, pszOutput);
					}
				}

				//std::cout << "name: " << info.name << std::endl << " id:" << info.id << std::endl;
				LogInstance()->rlog(IRecordLog::LOG_INFO,"name: %s,id: %s",info.name,info.id);
				audioDevices.push_back(info);
			}
			
			CoUninitialize();
			return audioDevices;
		}

		std::vector<Speaker> GetSpeakers() {
			std::vector<Speaker> speakers;
			std::vector<AudioDeviceInfo> audioDevices = GetAudioDevice(false);
			for_each(audioDevices.begin(), audioDevices.end(), [&](AudioDeviceInfo& audioDeviceInfo) {
				Speaker speaker;
				speaker.audioDeviceInfo = audioDeviceInfo;
				speakers.push_back(speaker);
			});
			
			return speakers;
		}

		std::vector<Microphone> GetMicrophones() {
			std::vector<Microphone> microphones;
			std::vector<AudioDeviceInfo> audioDevices = GetAudioDevice(true);
			for_each(audioDevices.begin(),audioDevices.end(), [&microphones](AudioDeviceInfo& audioDeviceInfo) {
				Microphone microphone;
				microphone.audioDeviceInfo = audioDeviceInfo;
				microphones.push_back(microphone);
			});

			return microphones;
		}
	}//namespace RecordCapture
}//namespace RL