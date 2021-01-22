#pragma once

#ifdef MEDIASTREAMER_EXPORTS
#define MEDIASTREAMER_API __declspec(dllexport)
#else
#define MEDIASTREAMER_API __declspec(dllimport)
#endif

#include <stdio.h>
#include <stdint.h>

enum WinMediaStreamerType
{
	WIN_MEDIA_STREAMER_SLK = 0,
	WIN_MEDIA_STREAMER_MULTISOURCE = 1,
	WIN_MEDIA_STREAMER_ANIMATEDIMAGE = 2,
};

enum {
	WIN_VIDEOFRAME_RAWTYPE_I420 = 0x0001,
	WIN_VIDEOFRAME_RAWTYPE_NV12 = 0x0002,
	WIN_VIDEOFRAME_RAWTYPE_NV21 = 0x0003,
	WIN_VIDEOFRAME_RAWTYPE_BGRA = 0x0004,
	WIN_VIDEOFRAME_RAWTYPE_RGBA = 0x0005,
	WIN_VIDEOFRAME_RAWTYPE_ARGB = 0x0006,
	WIN_VIDEOFRAME_RAWTYPE_ABGR = 0x0007,
};

enum {
	WIN_VIDEO_HARD_ENCODE = 0,
	WIN_VIDEO_SOFT_ENCODE = 1
};

struct WinVideoOptions
{
	bool hasVideo;

	int videoEncodeType;

	int videoWidth;
	int videoHeight;
	int videoFps;
	int videoRawType;

	int videoProfile;
	int videoBitRate; //k
	int encodeMode; //0:VBR or 1:CBR
	int maxKeyFrameIntervalMs;

	int networkAdaptiveMinFps;
	int networkAdaptiveMinVideoBitrate;
	bool isEnableNetworkAdaptive;

	int quality; //[-5, 5]:CRF
	bool bStrictCBR;
	int deblockingFilterFactor; //[-6, 6] -6 light filter, 6 strong

	int rotation;

	WinVideoOptions()
	{
		hasVideo = true;

		videoEncodeType = WIN_VIDEO_SOFT_ENCODE;

		videoWidth = 480;
		videoHeight = 640;
		videoFps = 20;
		videoRawType = WIN_VIDEOFRAME_RAWTYPE_I420;

		videoProfile = 0;
		videoBitRate = 500;
		encodeMode = 0;
		maxKeyFrameIntervalMs = 3000;

		networkAdaptiveMinFps = 12;
		networkAdaptiveMinVideoBitrate = 200;
		isEnableNetworkAdaptive = true;

		quality = 0;
		bStrictCBR = false;
		deblockingFilterFactor = 0;

		rotation = 0;
	}
};

struct WinAudioOptions
{
	bool hasAudio;

	int audioSampleRate;
	int audioNumChannels;
	int audioBitRate;//k

	bool isRealTime;
	bool isExternalAudioInput;

	WinAudioOptions()
	{
		hasAudio = true;

		audioSampleRate = 44100;
		audioNumChannels = 1;
		audioBitRate = 32; //high:64

		isRealTime = false;
		isExternalAudioInput = false;
	}
};

struct WinYUVVideoFrame
{
#define YUV_NUM_DATA_POINTERS 3
	uint8_t *data[YUV_NUM_DATA_POINTERS];
	int linesize[YUV_NUM_DATA_POINTERS];

	int planes;

	int width;
	int height;

	uint64_t pts; //ms
	uint64_t duration; //ms

	int rotation;

	int videoRawType;

	WinYUVVideoFrame()
	{
		for (int i = 0; i<YUV_NUM_DATA_POINTERS; i++)
		{
			data[i] = NULL;
			linesize[i] = 0;
		}

		planes = 0;

		width = 0;
		height = 0;

		pts = 0;
		duration = 0ll;

		rotation = 0;
		videoRawType = WIN_VIDEOFRAME_RAWTYPE_I420;
	}
};

struct WinVideoFrame
{
	uint8_t *data;
	int frameSize;

	int width;
	int height;

	uint64_t pts; //ms
	uint64_t duration; //ms

	int rotation;

	int videoRawType;

	bool isLastVideoFrame;

	bool isKeepFullContent;

	WinVideoFrame()
	{
		data = NULL;
		frameSize = 0;

		width = 0;
		height = 0;

		pts = 0;
		duration = 0ll;

		rotation = 0;
		videoRawType = WIN_VIDEOFRAME_RAWTYPE_NV21;

		isLastVideoFrame = false;

		isKeepFullContent = true;
	}
};

struct WinAudioFrame
{
	uint8_t *data;
	int frameSize;

	float duration; //ms

	uint64_t pts; //ms

	int sampleRate;
	int channels;
	int bitsPerChannel;

	bool isBigEndian;

	WinAudioFrame()
	{
		data = NULL;
		frameSize = 0;
		duration = 0.0f;
		pts = 0;

		sampleRate = 48000;
		channels = 2;
		bitsPerChannel = 16;

		isBigEndian = false;
	}
};

enum win_media_streamer_event_type {
	WIN_MEDIA_STREAMER_CONNECTING = 0,
	WIN_MEDIA_STREAMER_CONNECTED = 1,
	WIN_MEDIA_STREAMER_STREAMING = 2,
	WIN_MEDIA_STREAMER_ERROR = 3,
	WIN_MEDIA_STREAMER_INFO = 4,
	WIN_MEDIA_STREAMER_END = 5,
	WIN_MEDIA_STREAMER_PAUSED = 6,
};

enum win_media_streamer_info_type {
	WIN_MEDIA_STREAMER_INFO_ALREADY_CONNECTING = 1,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_DELAY_TIME = 2,
	WIN_MEDIA_STREAMER_INFO_ALREADY_ENDING = 3,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_REAL_BITRATE = 4,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_REAL_FPS = 5,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_DOWN_BITRATE = 6,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_UP_BITRATE = 7,
	WIN_MEDIA_STREAMER_INFO_PUBLISH_TIME = 8,

	WIN_MEDIA_STREAMER_INFO_AUDIO_INPUT_DATA_LOST = 100,
};

enum win_media_streamer_error_type {
	WIN_MEDIA_STREAMER_ERROR_UNKNOWN = -1,
	WIN_MEDIA_STREAMER_ERROR_CONNECT_FAIL = 0,
	WIN_MEDIA_STREAMER_ERROR_MUX_FAIL = 1,
	WIN_MEDIA_STREAMER_ERROR_COLORSPACECONVERT_FAIL = 2,
	WIN_MEDIA_STREAMER_ERROR_VIDEO_ENCODE_FAIL = 3,
	WIN_MEDIA_STREAMER_ERROR_AUDIO_CAPTURE_START_FAIL = 4,
	WIN_MEDIA_STREAMER_ERROR_AUDIO_ENCODE_FAIL = 5,
	WIN_MEDIA_STREAMER_ERROR_AUDIO_CAPTURE_STOP_FAIL = 6,
	WIN_MEDIA_STREAMER_ERROR_POOR_NETWORK = 7,
	WIN_MEDIA_STREAMER_ERROR_AUDIO_FILTER_FAIL = 8,
	WIN_MEDIA_STREAMER_ERROR_OPEN_VIDEO_ENCODER_FAIL = 9,
};

enum WIN_VIDEO_CAPTURE_TYPE
{
	WIN_GDI_DESKTOP_CAPTURE = 0,
	WIN_DSHOW_CAMERA_CAPTURE = 1,
};

class IWinMediaStreamerListener
{
public:
	virtual ~IWinMediaStreamerListener() {}

	virtual void onMediaStreamerConnecting() = 0;
	virtual void onMediaStreamerConnected() = 0;
	virtual void onMediaStreamerStreaming() = 0;
	virtual void onMediaStreamerPaused() = 0;
	virtual void onMediaStreamerError(int errorType) = 0;
	virtual void onMediaStreamerInfo(int infoType, int infoValue) = 0;
	virtual void onMediaStreamerEnd() = 0;
};

class IWinMediaStreamer
{
public:
	virtual void Release() = 0;

	virtual void initialize(const char* publishUrl, WinVideoOptions videoOptions, WinAudioOptions audioOptions, WinMediaStreamerType type = WIN_MEDIA_STREAMER_SLK, const char *backupDir = NULL) = 0;
	
	virtual void setListener(void(*listener)(void*, int, int, int), void* arg) = 0;
	virtual void setListener(IWinMediaStreamerListener *listener) = 0;
	
	virtual void inputVideoFrame(WinVideoFrame *inVideoFrame) = 0;
	virtual void inputYUVVideoFrame(WinYUVVideoFrame* inVideoFrame) = 0;

	virtual void inputAudioFrame(WinAudioFrame *inAudioFrame) = 0;
	virtual void inputFloatAudioFrame(float *data, int count, float duration, uint64_t pts, int sampleRate, int channels, int bitsPerChannel, bool isBigEndian) = 0;

	virtual void inputText(char* text, int size) = 0;

	virtual void start() = 0;

	virtual void resume() = 0;
	virtual void pause() = 0;

	virtual void stop(bool isCancle = false) = 0;

	virtual void enableAudio(bool isEnable) = 0;

	virtual void terminate() = 0;

	virtual void* getCore() = 0;
};

class IWinVideoCapturer
{
public:
	virtual void Release() = 0;

	virtual void initialize(WIN_VIDEO_CAPTURE_TYPE type, int videoWidth, int videoHeight, int videoFps) = 0;

	virtual void StartRecording() = 0;
	virtual void StopRecording() = 0;

	virtual void linkPreview(void* display) = 0;
	virtual void unlinkPreview() = 0;
	virtual void onPreviewResize(int w, int h) = 0;

	virtual void linkMediaStreamer(IWinMediaStreamer* winMediaStreamer) = 0;
	virtual void unlinkMediaStreamer() = 0;

	virtual void terminate() = 0;
};

extern "C" MEDIASTREAMER_API IWinMediaStreamer* _stdcall CreateWinMediaStreamerInstance();
extern "C" MEDIASTREAMER_API void _stdcall DestroyWinMediaStreamerInstance(IWinMediaStreamer** ppInstance);

extern "C" MEDIASTREAMER_API IWinVideoCapturer* _stdcall CreateWinVideoCapturerInstance();
extern "C" MEDIASTREAMER_API void _stdcall DestroyWinVideoCapturerInstance(IWinVideoCapturer** ppInstance);