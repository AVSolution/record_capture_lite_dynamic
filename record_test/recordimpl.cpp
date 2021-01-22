#include "recordimpl.h"
#include "../MediaStream/include/IWinMediaStreamer.h"

#ifdef _DEBUG
#pragma comment(lib,"../bind/RecordCapture.lib")
#pragma comment(lib,"../MediaStream/Debug/MediaStreamer.lib")
#else 
#pragma  comment(lib,"../bin/RecordCapture.lib")
#pragma comment(lib,"../MediaStream/Release/MediaStreamer.lib")
#endif

#define DEVMODE
#define Timer_intervel 5

CRecordImpl::CRecordImpl()
{
    logInstance = RL::RecordCapture::CreateRecordLog([]() {
        std::string appname = "/yuer/log/record/record.log";
        return RL::RecordCapture::GetLocalAppDataPath() + appname;
    });

    logInstance->rlog(RLOG::LOG_INFO, "%s","=====record running======");

    strAppName = "鱼耳";

    auto windows = RL::RecordCapture::GetWindows();
    for (auto &window : windows) {
        std::string name = window.Name;
        if (name.find(strAppName.data()) != std::string::npos) {
            yuerWindow = window;
            m_bFindYuer.store(true);
            break;
        }
    }
}

CRecordImpl::~CRecordImpl()
{
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

void CRecordImpl::startRecord(const std::string& path,VideoParam& videoParam,AudioParam &audioParam)
{
    //logInstance->rlog(RLOG::LOG_INFO,"start record Path:%s,VideoParam: %d,%d,%d,%d;AudioParam: %d,%d,%d,%d ",
    //                  path.toLocal8Bit().data(),videoParam.width,videoParam.height,videoParam.fps,videoParam.bitrate,
    //                  audioParam.sample,audioParam.channel,audioParam.depth,audioParam.bitrate);

    logInstance->rlog(RLOG::LOG_INFO,__FUNCTION__);
    pWinMediaStreamer = CreateWinMediaStreamerInstance();
    int widthDest = yuerWindow.Size.x;
    int heightDest = yuerWindow.Size.y;
    if(widthDest % 2) {
        widthDest -= 1;
    }
    if(heightDest % 2) {
        heightDest -= 1;
    }
    logInstance->rlog(RLOG::LOG_INFO,"record source width : %d, heigh: %d; destination width: %d,height: %d",
                      yuerWindow.Size.x,yuerWindow.Size.y,widthDest,heightDest);
    std::string appname = "/yuer/log/record/";

    const char* publishUrl = path.data();
    WinVideoOptions videoOptions;
    videoOptions.videoWidth = widthDest;
    videoOptions.videoHeight = heightDest;
    videoOptions.videoFps = videoParam.fps;
    videoOptions.videoBitRate = videoParam.bitrate;
	videoOptions.encodeMode = 0;
	videoOptions.videoProfile = 2;
    WinAudioOptions audioOptions;
    audioOptions.audioBitRate = audioParam.bitrate;
    audioOptions.audioSampleRate = audioParam.sample;
    audioOptions.audioNumChannels = audioParam.channel;
    audioOptions.isExternalAudioInput = true;
    pWinMediaStreamer->initialize(publishUrl, videoOptions, audioOptions,WIN_MEDIA_STREAMER_SLK,
                                  (RL::RecordCapture::GetLocalAppDataPath() + appname).c_str());
    pWinMediaStreamer->start();
    //pWinMediaStreamer->setListener(MediaStreamerTestListener,this);

    mixAudioBuffer = std::make_unique<CicleBuffer>(48000*2*2,0);
    mixAudioBuffer->flushBuffer();
    initVideoCapture();
    initSpeakAudioCapture();
    //initMicAudioCapture();
}

void CRecordImpl::stopRecord()
{
    //logInstance->rlog(RLOG::LOG_DEBUG,"RecordImpl::StopRecord");
    if(speakerGrabber)
        speakerGrabber->pause();
    if(microphoneGrabber)
        microphoneGrabber->pause();
    if(windowGrabber)
        windowGrabber->pause();

    if(pWinMediaStreamer) {
        pWinMediaStreamer->stop();
        pWinMediaStreamer->terminate();
        DestroyWinMediaStreamerInstance(std::addressof(pWinMediaStreamer));
        pWinMediaStreamer = nullptr;
    }
}

void CRecordImpl::initVideoCapture()
{
    static std::atomic<int> realcounter = 0;
    static auto onNewFramestart = std::chrono::high_resolution_clock::now();

    windowGrabber =
            RL::RecordCapture::CreateCaptureConfiguration( [&]() {
        std::vector<RL::RecordCapture::Window> filtereditems;
        filtereditems.push_back(yuerWindow);
        return filtereditems;
    })->onNewFrame([&](const RL::RecordCapture::Image &img, const RL::RecordCapture::Window &window) {
        realcounter.fetch_add(1);
        int nBufferLen = 4 * Width(window) * Height(window);
#ifdef DEVMODE1
        static FILE* pRecordFile = nullptr;
        if (nullptr == pRecordFile)
            fopen_s(&pRecordFile,"app.raw","wb");
        fwrite((void*)img.Data,nBufferLen, 1, pRecordFile);
#endif
        if (Timer_intervel * 1000 <= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count()) {
            auto fps = realcounter * 1.0 / Timer_intervel;
            realcounter = 0;
            onNewFramestart = std::chrono::high_resolution_clock::now();
            logInstance->rlog(RLOG::LOG_DEBUG, "window frame fps: %0.2f", fps);
        }
        //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<" onNewFrame. width: " <<Width(window)<<" height: "<<Height(window)<< std::endl;
        //to do.
        WinVideoFrame winVideoFrame;
        winVideoFrame.data = (uint8_t *)img.Data;
        winVideoFrame.frameSize = nBufferLen;
        winVideoFrame.width = window.Size.x;
        winVideoFrame.height = window.Size.y;
        winVideoFrame.pts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        winVideoFrame.videoRawType = WIN_VIDEOFRAME_RAWTYPE_BGRA;
        pWinMediaStreamer->inputVideoFrame(std::addressof(winVideoFrame));

    })->start_capturing();

    windowGrabber->setFrameChangeInterval(std::chrono::milliseconds(50));
}

//grabber default speaker sound.
void CRecordImpl::initSpeakAudioCapture()
{
    static std::atomic<int> realcounterAudio = 0;
    static auto onAudioFrameStart = std::chrono::high_resolution_clock::now();
    static std::unique_ptr<unsigned char[]> outAudioBuffer = nullptr;
    static std::unique_ptr<unsigned char[]> outAudioBufferTemp = std::make_unique<unsigned char[]>(0x800000);
    memset(outAudioBufferTemp.get(), 0, 0x800000);

    speakerGrabber =
            RL::RecordCapture::CreateCaptureConfiguration([&]() {
        auto speakers = RL::RecordCapture::GetSpeakers();
        return speakers;
    })->onAudioFrame([&](const RL::RecordCapture::AudioFrame &audioFrame) {
        int len = audioFrame.samples * audioFrame.channels * audioFrame.bytesPerSample;
        int len_s16 = len / 2;
        if (audioFrame.bytesPerSample == 2) {
            len_s16 = len;
        }
        if (outAudioBuffer == nullptr) {
            outAudioBuffer = std::make_unique<unsigned char []>(len_s16);
            memset(outAudioBuffer.get(), 0, len_s16);//save 16bit pcm source audio buffer.
        }
        if (audioFrame.bytesPerSample == 4) {
            RL::Util::convert32fToS16((int32_t*)audioFrame.buffer, len / sizeof(int32_t), (int16_t*)outAudioBuffer.get());
        }
        else if (audioFrame.bytesPerSample == 2) {//16 bit
            memcpy(outAudioBuffer.get(),audioFrame.buffer,len_s16);
        }

#ifdef DEVMODE1
        static FILE* pOutPutFile = nullptr;
        if (nullptr == pOutPutFile)
            pOutPutFile = fopen("speaker.pcm", "wb");
        if (audioFrame.buffer)
            fwrite(outAudioBuffer.get(), len_s16, 1, pOutPutFile);
#endif
        //mix microhone
        unsigned int readBufferLen = 0;
        if (mixAudioBuffer->readBuffer(outAudioBufferTemp.get(), len_s16, &readBufferLen)) {
            int nMixLen = len_s16 > readBufferLen ? readBufferLen : len_s16;
            RL::Util::MixerAddS16((int16_t*)outAudioBuffer.get(),(int16_t*)outAudioBufferTemp.get(), nMixLen / sizeof(int16_t));
        }
#ifdef DEVMODE
        static FILE* pOutPutFileMix = nullptr;
        if (nullptr == pOutPutFileMix)
			pOutPutFileMix = fopen("mix.pcm", "wb");
        if (audioFrame.buffer)
            fwrite(outAudioBuffer.get(), len_s16, 1, pOutPutFileMix);
#endif

        if(audioFrame.buffer)
        {
            WinAudioFrame winAudioFrame;
            winAudioFrame.data = outAudioBuffer.get();
            winAudioFrame.frameSize = len_s16;
            winAudioFrame.pts = audioFrame.renderTimeMs;
			winAudioFrame.sampleRate = audioFrame.samplesPerSec;
            pWinMediaStreamer->inputAudioFrame(&winAudioFrame);
        }

        //cout<<audioFrame.renderTimeMs<<" onAudioFrame."<<std::endl;
        realcounterAudio.fetch_add(1);
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onAudioFrameStart).count() > Timer_intervel * 1000) {
            auto fpsaudio = realcounterAudio * 1.0 / Timer_intervel;
            realcounterAudio = 0;
            onAudioFrameStart = std::chrono::high_resolution_clock::now();
            logInstance->rlog(RLOG::LOG_DEBUG, "speaker audio frame fps: %0.2f", fpsaudio);
        }
    })->start_capturing();
}

//grabber default microphone sound.
void CRecordImpl::initMicAudioCapture()
{
    static std::atomic<int> realcounterMic = 0;
    static auto onMicFrameStart = std::chrono::high_resolution_clock::now();
    static std::unique_ptr<unsigned char[]> outMicAudioBuffer = nullptr;

    microphoneGrabber =
            RL::RecordCapture::CreateCaptureConfiguration([&]() {
        auto microphones = RL::RecordCapture::GetMicrophones();
        return microphones;
    })->onAudioFrame([&](const RL::RecordCapture::AudioFrame &audioFrame) {
        //cout<<audioFrame.renderTimeMs<<" onAudioFrame."<<std::endl;
        realcounterMic.fetch_add(1);
        int len = audioFrame.samples * audioFrame.channels * audioFrame.bytesPerSample;
        int len_s16 = len / 2;
        if (audioFrame.bytesPerSample == 2) {
            len_s16 = len;
        }
        if (outMicAudioBuffer == nullptr) {
            outMicAudioBuffer = std::make_unique<unsigned char[]>(len_s16);
            memset(outMicAudioBuffer.get(), 0, len_s16);
        }
        if (audioFrame.bytesPerSample == 4)
            RL::Util::convert32fToS16((int32_t*)audioFrame.buffer, len / sizeof(int32_t), (int16_t*)outMicAudioBuffer.get());
        else if (audioFrame.bytesPerSample == 2 && audioFrame.channels == 2)
            memcpy(outMicAudioBuffer.get(), audioFrame.buffer, len_s16);

#ifdef DEVMODE
        static FILE* pOutPutFile = nullptr;
        if (nullptr == pOutPutFile)
            pOutPutFile = fopen("microphone.pcm", "wb");
        if (audioFrame.buffer)
            fwrite(outMicAudioBuffer.get(), len_s16, 1, pOutPutFile);
#endif
        mixAudioBuffer->writeBuffer((void*)outMicAudioBuffer.get(), len_s16);

        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onMicFrameStart).count() > Timer_intervel * 1000) {
            auto fpsaudio = realcounterMic * 1.0 / Timer_intervel;
            realcounterMic = 0;
            onMicFrameStart = std::chrono::high_resolution_clock::now();
            logInstance->rlog(RLOG::LOG_DEBUG, "mic audio frame fps: %0.2f", fpsaudio);
        }
    })->start_capturing();
}
