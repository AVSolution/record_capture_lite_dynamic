// record_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../record_capture/Capture.h"
#include "../record_capture/SCCommon.h"
#ifdef _DEBUG
#pragma comment(lib,"../bind/record_capture.lib")
#else 
#pragma  comment(lib,"../bin/record_capture.lib")
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

void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size)
{
	assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(SL::Screen_Capture::ImageBGRA)));
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
		imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
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
	std::unique_ptr<IRecordCapture> recordcapture(static_cast<IRecordCapture*>(getRecordInstance()));
	recordcapture->initialization();
	recordcapture->startRecord();
	recordcapture->stopRecord();

    std::cout << "Hello World!\n";
	*/
	//record windows .
	std::atomic<int> realcounter = 0;
	std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager>  framegrabber =
		SL::Screen_Capture::CreateCaptureConfiguration( []() {
		auto windows = SL::Screen_Capture::GetWindows();
		decltype(windows) filtereditems;
		std::string strchterm = "Óã¶ú";
		for (auto &window : windows) {
			std::string name = window.Name;
			if (name.find(strchterm) != string::npos) {
				filtereditems.push_back(window);
			}
		}
		return filtereditems;
	})->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) {
// 		static FILE* pRecordFile = nullptr;
// 		if (nullptr == pRecordFile)
// 			fopen_s(&pRecordFile,"dmp.bgra","wb");
// 		int nBufferLen = 4 * Width(window) * Height(window);
// 		fwrite((void*)img.Data,nBufferLen, 1, pRecordFile);
		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string("WINNEW_") + std::string(".jpg");
		auto size = Width(img) * Height(img) * sizeof(SL::Screen_Capture::ImageBGRA);

		auto imgbuffer(std::make_unique<unsigned char[]>(size));
		ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
		tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());

		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<" onNewFrame. width: " <<Width(window)<<" height: "<<Height(window)<< std::endl;

	})->onFrameChanged([](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) {
		//cout<<"onFrameChanged."<<endl;
	})->onMouseChanged([](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::MousePoint &mousepoint) {
		//cout<<"onMouseChanged."<<endl;
	})->start_capturing();

	std::cout<<"Sleep 1 second"<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	framegrabber->setFrameChangeInterval(std::chrono::milliseconds(50));
	framegrabber->setMouseChangeInterval(std::chrono::milliseconds(50));

	std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
