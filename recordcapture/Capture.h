#ifndef __CAPTRUE_H__
#define __CAPTRUE_H__

#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#define SC_LITE_DLL
#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL)
#define SC_LITE_EXTERN __declspec(dllexport)
#else
#define SC_LITE_EXTERN
#endif
#else
#define SC_LITE_EXTERN
#endif

class SC_LITE_EXTERN IRecordCaptureCallback {
public:
	virtual ~IRecordCaptureCallback() { ; }

	virtual void onSpeakerAudio() = 0;
	virtual void onWindowFrame() = 0;
};

class IRecordCapture {
public:
	virtual ~IRecordCapture() { ; }

	virtual void initialization() = 0;
	virtual void startRecord() = 0;
	virtual void stopRecord() = 0;
};

extern "C" {
	SC_LITE_EXTERN void* getRecordInstance();
	SC_LITE_EXTERN void releaseRecordInstance(void* pInstance);
}

namespace RL {
	namespace RecordCapture {
		struct SC_LITE_EXTERN Point {
			int x;
			int y;
		};
		struct SC_LITE_EXTERN MousePoint {
			Point Position;
			Point HotSpot;
		};
		struct SC_LITE_EXTERN Window {
			size_t Handle;
			Point Position;

			Point Size;
			// Name will always be lower case. It is converted to lower case internally by the library for comparisons
			char Name[128] = { 0 };
		};
		struct SC_LITE_EXTERN Monitor {
			int Id = INT32_MAX;
			int Index = INT32_MAX;
			int Adapter = INT32_MAX;
			int Height = 0;
			int Width = 0;
			int OriginalHeight = 0;
			int OriginalWidth = 0;
			// Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
			// monitors around so this affects their offset.
			int OffsetX = 0;
			int OffsetY = 0;
			int OriginalOffsetX = 0;
			int OriginalOffsetY = 0;
			char Name[128] = { 0 };
			float Scaling = 1.0f;
		};
		struct ImageRect {
			ImageRect() : ImageRect(0, 0, 0, 0) {}
			ImageRect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
			int left;
			int top;
			int right;
			int bottom;
			bool Contains(const ImageRect &a) const { return left <= a.left && right >= a.right && top <= a.top && bottom >= a.bottom; }
		};
		struct ImageBGRA {
			unsigned char B, G, R, A;
		};
		struct Image {
			ImageRect Bounds;
			int BytesToNextRow = 0;
			bool isContiguous = false;
			// alpha is always unused and might contain garbage
			const ImageBGRA *Data = nullptr;
		};

		struct AudioDeviceInfo {
			char name[128] = { 0 };
			char id[128] = { 0 };
		};
		struct Microphone {
			AudioDeviceInfo audioDeviceInfo;
		};
		struct Speaker {
			AudioDeviceInfo audioDeviceInfo;
		};
		struct AudioFrame {
			int 	samples = 0;
			int 	bytesPerSample = 0;
			int 	channels = 0;
			int 	samplesPerSec = 0;
			void * 	buffer = nullptr;
			int64_t 	renderTimeMs = 0;
		};

		// index to self in the GetMonitors() function
		SC_LITE_EXTERN int Index(const Monitor &mointor);
		// unique identifier
		SC_LITE_EXTERN int Id(const Monitor &mointor);
		SC_LITE_EXTERN int Adapter(const Monitor &mointor);
		SC_LITE_EXTERN int OffsetX(const Monitor &mointor);
		SC_LITE_EXTERN int OffsetY(const Monitor &mointor);
		SC_LITE_EXTERN void OffsetX(Monitor &mointor, int x);
		SC_LITE_EXTERN void OffsetY(Monitor &mointor, int y);
		SC_LITE_EXTERN int OffsetX(const Window &mointor);
		SC_LITE_EXTERN int OffsetY(const Window &mointor);
		SC_LITE_EXTERN void OffsetX(Window &mointor, int x);
		SC_LITE_EXTERN void OffsetY(Window &mointor, int y);
		SC_LITE_EXTERN const char *Name(const Monitor &mointor);
		SC_LITE_EXTERN const char *Name(const Window &mointor);
		SC_LITE_EXTERN int Height(const Monitor &mointor);
		SC_LITE_EXTERN int Width(const Monitor &mointor);
		SC_LITE_EXTERN void Height(Monitor &mointor, int h);
		SC_LITE_EXTERN void Width(Monitor &mointor, int w);
		SC_LITE_EXTERN int Height(const Window &mointor);
		SC_LITE_EXTERN int Width(const Window &mointor);
		SC_LITE_EXTERN void Height(Window &mointor, int h);
		SC_LITE_EXTERN void Width(Window &mointor, int w);
		SC_LITE_EXTERN int Height(const Image &img);
		SC_LITE_EXTERN int Width(const Image &img);
		SC_LITE_EXTERN int X(const Point &p);
		SC_LITE_EXTERN int Y(const Point &p);

		// the start of the image data, this is not guarenteed to be contiguous.
		SC_LITE_EXTERN const ImageBGRA *StartSrc(const Image &img);
		SC_LITE_EXTERN const ImageBGRA *GotoNextRow(const Image &img, const ImageBGRA *current);
		SC_LITE_EXTERN bool isDataContiguous(const Image &img);
		/*
			this is the ONLY funcion for pulling data out of the Image object and is layed out here in the header so that
			users can see how to extra data and convert it to their own needed format. Initially, I included custom extract functions
			but this is beyond the scope of this library. You must copy the image data if you want to use it as the library owns the Image Data.
		*/
		inline void Extract(const Image &img, unsigned char *dst, size_t dst_size)
		{
			assert(dst_size >= static_cast<size_t>(Width(img) * Height(img) * sizeof(ImageBGRA)));
			auto startdst = dst;
			auto startsrc = StartSrc(img);
			if (isDataContiguous(img)) { // no padding, the entire copy can be a single memcpy call
				memcpy(startdst, startsrc, Width(img) * Height(img) * sizeof(ImageBGRA));
			}
			else {
				for (auto i = 0; i < Height(img); i++) {
					memcpy(startdst, startsrc, sizeof(ImageBGRA) * Width(img));
					startdst += sizeof(ImageBGRA) * Width(img); // advance to the next row
					startsrc = GotoNextRow(img, startsrc);      // advance to the next row
				}
			}
		}

		class Timer {
			using Clock =
				std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

			std::chrono::microseconds Duration;
			Clock::time_point Deadline;

		public:
			template <typename Rep, typename Period>
			Timer(const std::chrono::duration<Rep, Period> &duration)
				: Duration(std::chrono::duration_cast<std::chrono::microseconds>(duration)), Deadline(Clock::now() + Duration)
			{
			}
			void start() { Deadline = Clock::now() + Duration; }
			void wait()
			{
				const auto now = Clock::now();
				if (now < Deadline) {
					std::this_thread::sleep_for(Deadline - now);
				}
			}
			std::chrono::microseconds duration() const { return Duration; }
		};

		// will return all attached monitors
		SC_LITE_EXTERN std::vector<Monitor> GetMonitors();
		// will return all windows
		SC_LITE_EXTERN std::vector<Window> GetWindows();
		// will return all speakers
		SC_LITE_EXTERN std::vector<Speaker> GetSpeakers();
		//will return all microphones
		SC_LITE_EXTERN std::vector<Microphone> GetMicrophones();
		//will return app local data Dir
		SC_LITE_EXTERN std::string GetLocalAppDataPath();

		typedef std::function<void(const RL::RecordCapture::Image &img, const Window &window)> WindowCaptureCallback;
		typedef std::function<void(const RL::RecordCapture::Image &img, const Monitor &monitor)> ScreenCaptureCallback;
		typedef std::function<void(const RL::RecordCapture::Image *img, const MousePoint &mousepoint)> MouseCallback;
		typedef std::function<void(const RL::RecordCapture::AudioFrame &audioFrame)> SpeakerCaptureCallback;
		typedef std::function<void(const RL::RecordCapture::AudioFrame &audioFrame)> MicrophoneCaptureCallback;
		typedef std::function<std::vector<Monitor>()> MonitorCallback;
		typedef std::function<std::vector<Window>()> WindowCallback;
		typedef std::function<std::vector<Microphone>()> MicrophoneCallback;
		typedef std::function<std::vector<Speaker>()> SpeakerCallback;
		typedef std::function<std::string()> LogCallBack;

		class SC_LITE_EXTERN IScreenCaptureManager {
		public:
			virtual ~IScreenCaptureManager() {}

			// Used by the library to determine the callback frequency
			template <class Rep, class Period> void setFrameChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
			{
				setFrameChangeInterval(std::make_shared<Timer>(rel_time));
			}
			// Used by the library to determine the callback frequency
			template <class Rep, class Period> void setMouseChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
			{
				setMouseChangeInterval(std::make_shared<Timer>(rel_time));
			}

			virtual void setFrameChangeInterval(const std::shared_ptr<Timer> &timer) = 0;
			virtual void setMouseChangeInterval(const std::shared_ptr<Timer> &timer) = 0;

			// Will pause all capturing
			virtual void pause() = 0;
			// Will return whether the library is paused
			virtual bool isPaused() const = 0;
			// Will resume all capturing if paused, otherwise has no effect
			virtual void resume() = 0;
		};

		template <typename CAPTURECALLBACK> class ICaptureConfiguration {
		public:
			virtual ~ICaptureConfiguration() {}
			// When a new frame is available the callback is invoked
			virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onNewFrame(const CAPTURECALLBACK &cb) = 0;
			// When a change in a frame is detected, the callback is invoked
			virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onFrameChanged(const CAPTURECALLBACK &cb) = 0;
			// When a mouse image changes or the mouse changes position, the callback is invoked.
			virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onMouseChanged(const MouseCallback &cb) = 0;
			// start capturing
			virtual std::shared_ptr<IScreenCaptureManager> start_capturing() = 0;
		};

		template <typename AUDIOCAPTURECALLBACK> class IAudioCaptureConfiguration {
		public:
			virtual ~IAudioCaptureConfiguration(){}
			//when a new audio frame is available the callback is invoked.
			virtual std::shared_ptr<IAudioCaptureConfiguration<AUDIOCAPTURECALLBACK>> onAudioFrame(const AUDIOCAPTURECALLBACK &cb) = 0;
			//start capturing.
			virtual std::shared_ptr<IScreenCaptureManager> start_capturing() = 0;
		};

		//the log for RecordCapture module.
		class IRecordLog {
		public:
			enum {
				LOG_ERROR = 100,
				LOG_WARNING = 200,
				LOG_INFO = 300,
				LOG_DEBUG = 400
			};

		public :
			virtual ~IRecordLog() {}
			virtual void rlog(int log,const char* format,...) = 0;
		};

		// the callback of windowstocapture represents the list of monitors which should be captured. Users should return the list of monitors they want
		// to be captured
		SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> CreateCaptureConfiguration(const MonitorCallback &monitorstocapture);
		// the callback of windowstocapture represents the list of windows which should be captured. Users should return the list of windows they want to
		// to be captured
		SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> CreateCaptureConfiguration(const WindowCallback &windowstocapture);
		// the callback of speakerstocapture represents the list of speakers which should be captured. Users should return the list of speakers they want to
		// to be captured.here choose default speaker.the list can be empty.
		SC_LITE_EXTERN std::shared_ptr<IAudioCaptureConfiguration<SpeakerCaptureCallback>> CreateCaptureConfiguration(const SpeakerCallback &speakertocapture);
		// the callback of microphonetocapture represents the list of microphone which should be captured. Users should return the list of microphone they want to
		// to be captured
		SC_LITE_EXTERN std::shared_ptr<IAudioCaptureConfiguration<MicrophoneCaptureCallback>> CreateCaptureConfiguration(const MicrophoneCallback &microhponertocapture);

		//the log handle.
		SC_LITE_EXTERN std::shared_ptr<IRecordLog> CreateRecordLog(const LogCallBack &logcallback);
	}//namespace RecordCapture
}//namespace RL

#endif 

