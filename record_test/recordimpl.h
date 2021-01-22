#ifndef CRECORDIMPL_H
#define CRECORDIMPL_H

#include <memory>
#include <atomic>
#include <iostream>
#include "../recordcapture/Capture.h"
#include "CicleBuffer.h"

using RLOG = RL::RecordCapture::IRecordLog;
using RCAPMANAGER = RL::RecordCapture::IScreenCaptureManager;

struct VideoParam {
    int width;
    int height;
    int fps;
    int bitrate;
};

struct AudioParam {
    int sample;
    int channel;
    int depth;
    int bitrate;
};

class IWinMediaStreamer;
class CRecordImpl
{
public:
    static CRecordImpl* getInstance() {
        static CRecordImpl recordImpl;
        return &recordImpl;
    }
    std::shared_ptr<RLOG> logInstance = nullptr;

    ~CRecordImpl();

    bool findYuerApp() {return m_bFindYuer.load();}

	void startRecord(const std::string& path, VideoParam& videoParam, AudioParam &audioParam);
	void stopRecord();

private:
    CRecordImpl();

    inline void initVideoCapture();
    inline void initSpeakAudioCapture();
    inline void initMicAudioCapture();

private:
    std::shared_ptr<RCAPMANAGER> windowGrabber = nullptr;
    std::shared_ptr<RCAPMANAGER> speakerGrabber = nullptr;
    std::shared_ptr<RCAPMANAGER> microphoneGrabber = nullptr;
    IWinMediaStreamer* pWinMediaStreamer = nullptr;

    std::unique_ptr<CicleBuffer> mixAudioBuffer = nullptr;

    std::atomic<bool> m_bFindYuer = false;
    std::string strAppName;
    RL::RecordCapture::Window yuerWindow;
};

#endif // CRECORDIMPL_H
