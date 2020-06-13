// record_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../RecordCapture/Capture.h"
//#include "../RecordCapture/SCCommon.h"
#ifdef _DEBUG
#pragma comment(lib,"../bind/RecordCapture.lib")
#else 
#pragma  comment(lib,"../bin/RecordCapture.lib")
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <iostream>
#include <locale>
#include <string>
#include <thread>
#include <vector>

// THESE LIBRARIES ARE HERE FOR CONVINIENCE!! They are SLOW and ONLY USED FOR
// HOW THE LIBRARY WORKS!
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#define LODEPNG_COMPILE_PNG
#define LODEPNG_COMPILE_DISK
#include "lodepng.h"

using namespace std;

void ExtractAndConvertToRGBA(const RL::RecordCapture::Image &img, unsigned char *dst, size_t dst_size)
{
	assert(dst_size >= static_cast<size_t>(RL::RecordCapture::Width(img) * RL::RecordCapture::Height(img) * sizeof(RL::RecordCapture::ImageBGRA)));
	auto imgsrc = StartSrc(img);
	auto imgdist = dst;
	for (auto h = 0; h < Height(img); h++) {
		auto startimgsrc = imgsrc;
		for (auto w = 0; w < Width(img); w++) {
			*imgdist++ = imgsrc->R;
			*imgdist++ = imgsrc->G;
			*imgdist++ = imgsrc->B;
			*imgdist++ = 0; // alpha should be zero
			imgsrc++;
		}
		imgsrc = RL::RecordCapture::GotoNextRow(img, startimgsrc);
	}
}

class CDerive : public IRecordCaptureCallback {
public: 
	CDerive() {
		cout << "Derive ctor"<< endl;
	}

	~CDerive() {
		cout << "Derive dtor" << endl;
	}

	virtual void onSpeakerAudio() override {
		cout<<"CDerived onSpeakerAudio."<<endl;
	}
	virtual void onWindowFrame() override {
		cout << "CDerived OnWindowFrame." << endl;
	}
};

int main()
{
	/*
	IRecordCapture* pInstance = static_cast<IRecordCapture*>(getRecordInstance());
	pInstance->initialization();
	pInstance->startRecord();
	pInstance->stopRecord();

	releaseRecordInstance(static_cast<void*>(pInstance));
	pInstance = nullptr;
	*/
	
	/*
	std::unique_ptr<IRecorCapture> RecordCapture(static_cast<IRecordCapture*>(getRecordInstance()));
	RecordCapture->initialization();
	RecordCapture->startRecord();
	RecordCapture->stopRecord();

    std::cout << "Hello World!\n";
	*/
	//record windows .

	
	std::shared_ptr<RL::RecordCapture::IRecordLog> logInstance = RL::RecordCapture::CreateRecordLog([]() {
		
		std::string appname = "/yuer/log/record/record.log";
		return RL::RecordCapture::GetLocalAppDataPath() + appname;
	});

	using RLOG = RL::RecordCapture::IRecordLog;
	logInstance->rlog(RLOG::LOG_INFO, "%s","=====record running........");
	
	std::atomic<int> realcounter = 0;
	auto onNewFramestart = std::chrono::high_resolution_clock::now();

	std::atomic<int> realcounterAudio = 0;
	auto onAudioFrameStart = std::chrono::high_resolution_clock::now();
	std::shared_ptr<RL::RecordCapture::IScreenCaptureManager>  framegrabber =
		RL::RecordCapture::CreateCaptureConfiguration( [&]() {
		auto windows = RL::RecordCapture::GetWindows();
		decltype(windows) filtereditems;
		std::string strchterm = "Óã¶ú";
		for (auto &window : windows) {
			std::string name = window.Name;
			if (name.find(strchterm) != string::npos) {
				filtereditems.push_back(window);break;
			}
		}
		return filtereditems;
	})->onNewFrame([&](const RL::RecordCapture::Image &img, const RL::RecordCapture::Window &window) {
		realcounter.fetch_add(1);
		static FILE* pRecordFile = nullptr;
		if (nullptr == pRecordFile)
			fopen_s(&pRecordFile,"app.raw","wb");
		int nBufferLen = 4 * Width(window) * Height(window);
		fwrite((void*)img.Data,nBufferLen, 1, pRecordFile);
		if (10 <= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count()) {
			auto fps = realcounter * 1.0 / 10;
			realcounter = 0;
			onNewFramestart = std::chrono::high_resolution_clock::now();
			logInstance->rlog(RLOG::LOG_DEBUG, "window fps: %0.2f", fps);
		}
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<" onNewFrame. width: " <<Width(window)<<" height: "<<Height(window)<< std::endl;
	})
		->start_capturing();
		
	std::shared_ptr<RL::RecordCapture::IScreenCaptureManager> speakergrabber =
		RL::RecordCapture::CreateCaptureConfiguration([&]() {
		auto speakers = RL::RecordCapture::GetSpeakers();
		return speakers;
	})->onAudioFrame([&](const RL::RecordCapture::AudioFrame &audioFrame) {
		//cout<<audioFrame.renderTimeMs<<" onAudioFrame."<<std::endl;
		realcounterAudio.fetch_add(1);
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onAudioFrameStart).count() > 10 * 1000) {
			auto fpsaudio = realcounterAudio * 1.0 / 10;
			realcounterAudio = 0;
			onAudioFrameStart = std::chrono::high_resolution_clock::now();
			logInstance->rlog(RLOG::LOG_DEBUG, "audio frame fps: %0.2f", fpsaudio);
		}
		static FILE* pOutPutFile = nullptr;
		if (nullptr == pOutPutFile)
			pOutPutFile = fopen("speaker.pcm", "wb");
		if (audioFrame.buffer) {
			int len = audioFrame.samples * audioFrame.channels * audioFrame.bytesPerSample;
			fwrite(audioFrame.buffer, len, 1, pOutPutFile);
		}
	})
		->start_capturing();

	int i = 0;
	while (++i < 10) {

		logInstance->rlog(RLOG::LOG_DEBUG, "Sleep 2 seconds");
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	logInstance->rlog(RL::RecordCapture::IRecordLog::LOG_INFO, "%s", "=====record end........\n");




	/*
	std::cout<<"Sleep 1 second"<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	framegrabber->setFrameChangeInterval(std::chrono::milliseconds(50));
	framegrabber->setMouseChangeInterval(std::chrono::milliseconds(50));

	std::this_thread::sleep_for(std::chrono::seconds(5));
	*/
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .RLn file
