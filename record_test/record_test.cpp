// record_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../RecordCapture/Capture.h"
//#include "../RecordCapture/SCCommon.h"
#include "../MediaStream/include/IWinMediaStreamer.h"
#include "../MediaStream/include/AudioSampleFormatConvert.h"
#ifdef _DEBUG
#pragma comment(lib,"../bind/RecordCapture.lib")
#pragma comment(lib,"../MediaStream/Debug/MediaStreamer.lib")
#else 
#pragma  comment(lib,"../bin/RecordCapture.lib")
#pragma comment(lib,"../MediaStream/Release/MediaStreamer.lib")
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

#define  RECORD

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

#define VOLUMEMAX   32767
int SimpleCalculate_DB(short* pcmData, int sample)
{
	signed short ret = 0;
	if (sample > 0) {
		int sum = 0;
		signed short* pos = (signed short *)pcmData;
		for (int i = 0; i < sample; i++) {
			sum += abs(*pos);
			pos++;
		}
		ret = sum * 500.0 / (sample * VOLUMEMAX);
		if (ret >= 100) {
			ret = 100;
		}
	}
	return ret;
}

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

	using RLOG = RL::RecordCapture::IRecordLog;
	std::shared_ptr<RL::RecordCapture::IRecordLog> logInstance = RL::RecordCapture::CreateRecordLog([]() {
		
		std::string appname = "/yuer/log/record/record.log";
		return RL::RecordCapture::GetLocalAppDataPath() + appname;
	});

	logInstance->rlog(RLOG::LOG_INFO, "%s", "=====record running........");

	auto windows = RL::RecordCapture::GetWindows();
	decltype(windows) filtereditems;;
	std::string strchterm = "Óã¶ú";
	for (auto &window : windows) {
		std::string name = window.Name;
		if (name.find(strchterm) != string::npos) {
			filtereditems.push_back(window); break;
		}
	}
	int nWidth = 1920;
	int nHeight = 1080;
	if (filtereditems.size()) {
		nWidth = filtereditems[0].Size.x;
		nHeight = filtereditems[0].Size.y; 
		logInstance->rlog(RLOG::LOG_INFO, "source width: %d,height:%d", nWidth, nHeight);
		if (nWidth % 2)
			nWidth -= 1;
		if (nHeight % 2)
			nHeight -= 1;
		logInstance->rlog(RLOG::LOG_INFO, "Destination width: %d,height:%d",nWidth,nHeight);
	}
	
	IWinMediaStreamer* winMediaStreamer = CreateWinMediaStreamerInstance();
	auto mux_initialization = [=]() {
		const char* publishUrl = "C:\\tmp\\cris.mp4";
		WinVideoOptions videoOptions;
		videoOptions.videoWidth =  nWidth;
		videoOptions.videoHeight =  nHeight;
		videoOptions.videoFps = 10;
		videoOptions.videoBitRate = 1024;
		WinAudioOptions audioOptions;
		audioOptions.audioBitRate = 128;
		audioOptions.audioSampleRate = 48000;
		audioOptions.audioNumChannels = 2;
		audioOptions.isExternalAudioInput = true;
		std::string mediaLog = RL::RecordCapture::GetLocalAppDataPath() + "/yuer/log/record/";
		winMediaStreamer->initialize(publishUrl, videoOptions, audioOptions, WIN_MEDIA_STREAMER_SLK, mediaLog.c_str());
		winMediaStreamer->start();
	};
#ifdef RECORD
	mux_initialization();
#endif

	std::atomic<int> realcounter = 0;
	auto onNewFramestart = std::chrono::high_resolution_clock::now();

	std::shared_ptr<RL::RecordCapture::IScreenCaptureManager>  framegrabber =
		RL::RecordCapture::CreateCaptureConfiguration( [&]() {

		return filtereditems;
	})->onNewFrame([&](const RL::RecordCapture::Image &img, const RL::RecordCapture::Window &window) {
		realcounter.fetch_add(1);
		static FILE* pRecordFile = nullptr;
		if (nullptr == pRecordFile)
			fopen_s(&pRecordFile,"app.raw","wb");
		int nBufferLen = 4 * Width(window) * Height(window);
		fwrite((void*)img.Data,nBufferLen, 1, pRecordFile);
		WinVideoFrame winVideoFrame;
		winVideoFrame.data = (uint8_t *)img.Data;
		winVideoFrame.frameSize = nBufferLen;
		winVideoFrame.width = window.Size.x;
		winVideoFrame.height = window.Size.y;
		winVideoFrame.pts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		winVideoFrame.videoRawType = WIN_VIDEOFRAME_RAWTYPE_BGRA;
#ifdef RECORD
		winMediaStreamer->inputVideoFrame(std::addressof(winVideoFrame));
#endif
		if (10 <= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count()) {
			auto fps = realcounter * 1.0 / 10;
			realcounter = 0;
			onNewFramestart = std::chrono::high_resolution_clock::now();
			logInstance->rlog(RLOG::LOG_DEBUG, "window fps: %0.2f", fps);
		}
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<" onNewFrame. width: " <<Width(window)<<" height: "<<Height(window)<< std::endl;
	})
		->start_capturing();
	
	std::atomic<int> realcounterAudio = 0;
	auto onAudioFrameStart = std::chrono::high_resolution_clock::now();
	std::unique_ptr<unsigned char[]> outAudioBuffer = nullptr;

	std::shared_ptr<RL::RecordCapture::IScreenCaptureManager> speakergrabber =
		RL::RecordCapture::CreateCaptureConfiguration([&]() {
		auto speakers = RL::RecordCapture::GetSpeakers();
		return speakers;
	})->onAudioFrame([&](const RL::RecordCapture::AudioFrame &audioFrame) {
		//cout<<audioFrame.renderTimeMs<<" onAudioFrame."<<std::endl;
		int len = audioFrame.samples * audioFrame.channels * audioFrame.bytesPerSample;
		if (outAudioBuffer == nullptr) {
			outAudioBuffer = std::make_unique<unsigned char []>(len / 2);
		}
		
		for (int i = 0; i < len /4; i ++ ) {
			float ff = *(float*)((uint8_t*)audioFrame.buffer + i * 4);
			short ss = ff * 32768;
			memcpy(outAudioBuffer.get() + i * 2, &ss,2);
		}
		//src_float_to_short_array((float*)audioFrame.buffer, (short*)outAudioBuffer.get(), len / sizeof(float));
 		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onAudioFrameStart).count() > 10 * 1000) {
			auto fpsaudio = realcounterAudio * 1.0 / 10;
			realcounterAudio = 0;
			onAudioFrameStart = std::chrono::high_resolution_clock::now();
			logInstance->rlog(RLOG::LOG_DEBUG, "speaker audio frame fps: %0.2f", fpsaudio);
		}
		static FILE* pOutPutFile = nullptr;
		if (nullptr == pOutPutFile)
			pOutPutFile = fopen("speaker.pcm", "wb");
		if (audioFrame.buffer) {
			fwrite(outAudioBuffer.get(), len/2, 1, pOutPutFile);
			//fwrite(audioFrame.buffer,len,1,pOutPutFile);
			WinAudioFrame winAudioFrame;
			winAudioFrame.data = outAudioBuffer.get();
			winAudioFrame.frameSize = len / 2;
			winAudioFrame.pts = audioFrame.renderTimeMs;
#ifdef RECORD
			winMediaStreamer->inputAudioFrame(&winAudioFrame);
#endif
			//printf("%d\n", SimpleCalculate_DB((short*)outAudioBuffer.get(), audioFrame.samples));
		}
	})
		->start_capturing();
	
	std::atomic<int> realcounterMic = 0;
	auto onMicFrameStart = std::chrono::high_resolution_clock::now();
	std::unique_ptr<unsigned char[]> outMicAudioBuffer = nullptr;
	std::shared_ptr<RL::RecordCapture::IScreenCaptureManager> micgrabber =
		RL::RecordCapture::CreateCaptureConfiguration([&]() {
		auto microphones = RL::RecordCapture::GetMicrophones();
		return microphones;
	})->onAudioFrame([&](const RL::RecordCapture::AudioFrame &audioFrame) {
		//cout<<audioFrame.renderTimeMs<<" onAudioFrame."<<std::endl;
		int len = audioFrame.samples * audioFrame.channels * audioFrame.bytesPerSample;
		if (outMicAudioBuffer == nullptr) {
			outMicAudioBuffer = std::make_unique<unsigned char[]>(len / 2);
		}

		for (int i = 0; i < len / 4; i++) {
			float ff = *(float*)((uint8_t*)audioFrame.buffer + i * 4);
			short ss = ff * 32768;
			memcpy(outMicAudioBuffer.get() + i * 2, &ss, 2);
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onMicFrameStart).count() > 10 * 1000) {
			auto fpsaudio = realcounterMic * 1.0 / 10;
			realcounterMic = 0;
			onMicFrameStart = std::chrono::high_resolution_clock::now();
			logInstance->rlog(RLOG::LOG_DEBUG, "mic audio frame fps: %0.2f", fpsaudio);
		}
		static FILE* pOutPutFile = nullptr;
		if (nullptr == pOutPutFile)
			pOutPutFile = fopen("microphone.pcm", "wb");
		if (audioFrame.buffer) {
			fwrite(outMicAudioBuffer.get(), len / 2, 1, pOutPutFile);
			WinAudioFrame winMicAudioFrame;
			winMicAudioFrame.data = outMicAudioBuffer.get();
			winMicAudioFrame.frameSize = len / 2;
			winMicAudioFrame.pts = audioFrame.renderTimeMs;
#ifdef RECORD
			winMediaStreamer->inputAudioFrame(&winMicAudioFrame);
#endif
		}
	})->start_capturing();
	
	int i = 0;
	while (++i < 4) {
		logInstance->rlog(RLOG::LOG_DEBUG, "Sleep 2 seconds");
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	
	speakergrabber->pause();
	micgrabber->pause();
	framegrabber->pause();
	std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef RECORD
	winMediaStreamer->stop();
	winMediaStreamer->terminate();
	if (winMediaStreamer)
		DestroyWinMediaStreamerInstance(std::addressof(winMediaStreamer));
#endif

	logInstance->rlog(RL::RecordCapture::IRecordLog::LOG_INFO, "%s", "=====record end........\n");

	//system("pause");

	/*
	std::cout<<"Sleep 1 second"<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	framegrabber->setFrameChangeInterval(std::chrono::milliseconds(50));
	framegrabber->setMouseChangeInterval(std::chrono::milliseconds(50));

	std::this_thread::sleep_for(std::chrono::seconds(5));
	*/
}

void MediaStreamerTestListener(void* owner, int event, int ext1, int ext2)
{
	if (event == WIN_MEDIA_STREAMER_CONNECTING)
	{
		printf("WIN_MEDIA_STREAMER_CONNECTING \n");
	}
	else if (event == WIN_MEDIA_STREAMER_CONNECTED)
	{
		printf("WIN_MEDIA_STREAMER_CONNECTED \n");
	}
	else if (event == WIN_MEDIA_STREAMER_STREAMING)
	{
		printf("WIN_MEDIA_STREAMER_STREAMING \n");
	}
	else if (event == WIN_MEDIA_STREAMER_ERROR)
	{
		printf("WIN_MEDIA_STREAMER_ERROR ErrorType:%d \n", ext1);
	}
	else if (event == WIN_MEDIA_STREAMER_INFO)
	{
		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_REAL_BITRATE) {
			printf("Real Bitrate:%d \n", ext2);
		}

		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_REAL_FPS) {
			printf("Real Fps:%d \n", ext2);
		}

		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_DELAY_TIME) {
			printf("buffer cache duration : %d \n", ext2);
		}

		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_DOWN_BITRATE) {
			printf("down target bitrate to : %d \n", ext2);
		}

		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_UP_BITRATE) {
			printf("up target bitrate to : %d \n", ext2);
		}

		if (ext1 == WIN_MEDIA_STREAMER_INFO_PUBLISH_TIME) {
			printf("Record Time:%f S \n", (float)(ext2) / 10.0f);
		}
	}
	else if (event == WIN_MEDIA_STREAMER_END)
	{
		printf("WIN_MEDIA_STREAMER_END \n");
	}
	else if (event == WIN_MEDIA_STREAMER_PAUSED)
	{
		printf("WIN_MEDIA_STREAMER_PAUSED \n");
	}
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
