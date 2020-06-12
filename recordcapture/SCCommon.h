#pragma once

#ifdef _WIN32
#include <Unknwn.h>
#endif

#include "Capture.h"
#include <atomic>
#include <thread>

// this is INTERNAL DO NOT USE!
namespace RL {
namespace recordcapture {

    inline bool operator==(const ImageRect &a, const ImageRect &b)
    {
        return b.left == a.left && b.right == a.right && b.top == a.top && b.bottom == a.bottom;
    }
	int Height(const ImageRect &rect);
	int Width(const ImageRect &rect);
    const ImageRect &Rect(const Image &img);

    template <typename F, typename M, typename W> struct CaptureData {
        std::shared_ptr<Timer> FrameTimer;
        F OnNewFrame;
        F OnFrameChanged;
        std::shared_ptr<Timer> MouseTimer;
        M OnMouseChanged;
        W getThingsToWatch;
    };
	template <typename F,typename W> struct AudioCaptureData {
		std::shared_ptr<Timer> FrameTimer;
		F onAudioFrame;
		W getThingsToWatch;
	};
    struct CommonData {
        // Used to indicate abnormal error condition
        std::atomic<bool> UnexpectedErrorEvent;
        // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate
        // the duplication interface
        std::atomic<bool> ExpectedErrorEvent;
        // Used to signal to threads to exit
        std::atomic<bool> TerminateThreadsEvent;
        std::atomic<bool> Paused;
    };

    struct Thread_Data {
		using CaptureMonitorType = CaptureData<ScreenCaptureCallback, MouseCallback, MonitorCallback>;
		using CaptureWindowType = CaptureData<WindowCaptureCallback, MouseCallback, WindowCallback>;
		using AudioSpeakerType = AudioCaptureData<SpeakerCaptureCallback, SpeakerCallback>;
		using AudioMicrophoneType = AudioCaptureData<MicrophoneCaptureCallback, MicrophoneCallback>;
        CaptureMonitorType ScreenCaptureData;
        CaptureWindowType WindowCaptureData;
		AudioSpeakerType SpeakerCaptureData;
		AudioMicrophoneType MicrophoneCaptureData;
        CommonData CommonData_;
    };

    class BaseFrameProcessor {
      public:
        std::shared_ptr<Thread_Data> Data;
        std::unique_ptr<unsigned char[]> ImageBuffer;
        int ImageBufferSize = 0;
        bool FirstRun = true;
    };

    enum DUPL_RETURN { DUPL_RETURN_SUCCESS = 0, DUPL_RETURN_ERROR_EXPECTED = 1, DUPL_RETURN_ERROR_UNEXPECTED = 2 };
    Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scale);
    Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scale);
    SC_LITE_EXTERN bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor);
    SC_LITE_EXTERN Image CreateImage(const ImageRect &imgrect, int rowpadding, const ImageBGRA *data);
    // this function will copy data from the src into the dst. The only requirement is that src must not be larger than dst, but it can be smaller
    // void Copy(const Image& dst, const Image& src);

    SC_LITE_EXTERN std::vector<ImageRect> GetDifs(const Image &oldimg, const Image &newimg);
    template <class F, class T, class C>
    void ProcessCapture(const F &data, T &base, const C &mointor, const unsigned char *startsrc, int srcrowstride)
    {
        ImageRect imageract;
        imageract.left = 0;
        imageract.top = 0;
        imageract.bottom = Height(mointor);
        imageract.right = Width(mointor);
        const auto sizeofimgbgra = static_cast<int>(sizeof(ImageBGRA));
        const auto startimgsrc = reinterpret_cast<const ImageBGRA *>(startsrc);
        auto dstrowstride = sizeofimgbgra * Width(mointor);
        if (data.OnNewFrame) { // each frame we still let the caller know if asked for
            auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
            wholeimg.isContiguous = dstrowstride == srcrowstride;
            data.OnNewFrame(wholeimg, mointor);
        }
        if (data.OnFrameChanged) { // difs are needed!
            if (base.FirstRun) {
                // first time through, just send the whole image
                auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
                wholeimg.isContiguous = dstrowstride == srcrowstride;
                data.OnFrameChanged(wholeimg, mointor);
                base.FirstRun = false;
            }
            else {
                // user wants difs, lets do it!
                auto newimg = CreateImage(imageract, srcrowstride - dstrowstride, startimgsrc);
                auto oldimg = CreateImage(imageract, 0, reinterpret_cast<const ImageBGRA *>(base.ImageBuffer.get()));
                auto imgdifs = GetDifs(oldimg, newimg);

                for (auto &r : imgdifs) {
                    auto leftoffset = r.left * sizeofimgbgra;
                    auto thisstartsrc = startsrc + leftoffset + (r.top * srcrowstride);

                    auto difimg = CreateImage(r, srcrowstride, reinterpret_cast<const ImageBGRA *>(thisstartsrc));
                    difimg.isContiguous = false;
                    data.OnFrameChanged(difimg, mointor);
                }
            }
            auto startdst = base.ImageBuffer.get();
            if (dstrowstride == srcrowstride) { // no need for multiple calls, there is no padding here
                memcpy(startdst, startsrc, dstrowstride * Height(mointor));
            }
            else {
                for (auto i = 0; i < Height(mointor); i++) {
                    memcpy(startdst + (i * dstrowstride), startsrc + (i * srcrowstride), dstrowstride);
                }
            }
        }
    }


	/* Oh no I have my own com pointer class, the world is ending, how dare you
	 * write your own! */

	template<class T> class ComPtr {

	protected:
		T *ptr;

		inline void Kill()
		{
			if (ptr)
				ptr->Release();
		}

		inline void Replace(T *p)
		{
			if (ptr != p) {
				if (p)
					p->AddRef();
				if (ptr)
					ptr->Release();
				ptr = p;
			}
		}

	public:
		inline ComPtr() : ptr(nullptr) {}
		inline ComPtr(T *p) : ptr(p)
		{
			if (ptr)
				ptr->AddRef();
		}
		inline ComPtr(const ComPtr<T> &c) : ptr(c.ptr)
		{
			if (ptr)
				ptr->AddRef();
		}
		inline ComPtr(ComPtr<T> &&c) : ptr(c.ptr) { c.ptr = nullptr; }
		inline ~ComPtr() { Kill(); }

		inline void Clear()
		{
			if (ptr) {
				ptr->Release();
				ptr = nullptr;
			}
		}

		inline ComPtr<T> &operator=(T *p)
		{
			Replace(p);
			return *this;
		}

		inline ComPtr<T> &operator=(const ComPtr<T> &c)
		{
			Replace(c.ptr);
			return *this;
		}

		inline ComPtr<T> &operator=(ComPtr<T> &&c)
		{
			if (&ptr != &c.ptr) {
				Kill();
				ptr = c.ptr;
				c.ptr = nullptr;
			}

			return *this;
		}

		inline T *Detach()
		{
			T *out = ptr;
			ptr = nullptr;
			return out;
		}

		inline void CopyTo(T **out)
		{
			if (out) {
				if (ptr)
					ptr->AddRef();
				*out = ptr;
			}
		}

		inline ULONG Release()
		{
			ULONG ref;

			if (!ptr)
				return 0;
			ref = ptr->Release();
			ptr = nullptr;
			return ref;
		}

		inline T **Assign()
		{
			Clear();
			return &ptr;
		}
		inline void Set(T *p)
		{
			Kill();
			ptr = p;
		}

		inline T *Get() const { return ptr; }

		inline T **operator&() { return Assign(); }

		inline operator T *() const { return ptr; }
		inline T *operator->() const { return ptr; }

		inline bool operator==(T *p) const { return ptr == p; }
		inline bool operator!=(T *p) const { return ptr != p; }

		inline bool operator!() const { return !ptr; }
	};
	
	template<typename T> class CoTaskMemPtr {
		T *ptr;

		inline void Clear()
		{
			if (ptr)
				CoTaskMemFree(ptr);
		}

	public:
		inline CoTaskMemPtr() : ptr(NULL) {}
		inline CoTaskMemPtr(T *ptr_) : ptr(ptr_) {}
		inline ~CoTaskMemPtr() { Clear(); }

		inline operator T *() const { return ptr; }
		inline T *operator->() const { return ptr; }

		inline const T *Get() const { return ptr; }

		inline CoTaskMemPtr &operator=(T *val)
		{
			Clear();
			ptr = val;
			return *this;
		}

		inline T **operator&()
		{
			Clear();
			ptr = NULL;
			return &ptr;
		}
	};

} // namespace recordcapture
} // namespace RL
