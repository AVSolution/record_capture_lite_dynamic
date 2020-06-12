#include "ThreadManager.h"
#include <assert.h>
#include <algorithm>

RL::recordcapture::ThreadManager::ThreadManager()
{
}
RL::recordcapture::ThreadManager::~ThreadManager()
{
    Join();
}

void RL::recordcapture::ThreadManager::Init(const std::shared_ptr<Thread_Data>& data)
{
    assert(m_ThreadHandles.empty());

    if (data->ScreenCaptureData.getThingsToWatch) {
        auto monitors = data->ScreenCaptureData.getThingsToWatch();
        auto mons = GetMonitors();
        for (auto& m : monitors) {
            assert(isMonitorInsideBounds(mons, m));
        }

        m_ThreadHandles.resize(monitors.size() + (data->ScreenCaptureData.OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed

        for (size_t i = 0; i < monitors.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&RL::recordcapture::RunCaptureMonitor, data, monitors[i]);
        }
        if (data->ScreenCaptureData.OnMouseChanged) {
            m_ThreadHandles.back() = std::thread([data] {
                RL::recordcapture::RunCaptureMouse(data);
            });
        }

    }
    else if (data->WindowCaptureData.getThingsToWatch) {
        auto windows = data->WindowCaptureData.getThingsToWatch();
        m_ThreadHandles.resize(windows.size() + (data->WindowCaptureData.OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed
        for (size_t i = 0; i < windows.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&RL::recordcapture::RunCaptureWindow, data, windows[i]);
        }
        if (data->WindowCaptureData.OnMouseChanged) {
            m_ThreadHandles.back() = std::thread([data] {
                RL::recordcapture::RunCaptureMouse(data);
            });
        }
    }
	else if (data->SpeakerCaptureData.getThingsToWatch || data->SpeakerCaptureData.onAudioFrame) {
		auto speakers = data->SpeakerCaptureData.getThingsToWatch();
		m_ThreadHandles.resize(speakers.size());
		for (size_t i = 0; i < speakers.size(); ++i)
			m_ThreadHandles[i] = std::thread(&RL::recordcapture::RunCaptureSpeaker, data, speakers[i]);
	}
	else if (data->MicrophoneCaptureData.getThingsToWatch && data->MicrophoneCaptureData.onAudioFrame) {
		auto microphones = data->MicrophoneCaptureData.getThingsToWatch();//the microphone list only your selected microphone.
		m_ThreadHandles.resize(microphones.size());
		for (size_t i = 0; i < microphones.size(); ++i)
			m_ThreadHandles[i] = std::thread(&RL::recordcapture::RunCaptureMicrophone, data, microphones[i]);
	}
}

void RL::recordcapture::ThreadManager::Join()
{
    for (auto& t : m_ThreadHandles) {
        if (t.joinable()) {
            if (t.get_id() == std::this_thread::get_id()) {
                t.detach();// will run to completion
            }
            else {
                t.join();
            }
        }
    }
    m_ThreadHandles.clear();
}
