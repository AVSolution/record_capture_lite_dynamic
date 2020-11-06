#ifndef __RESAMPLERATE_H__
#define __RESAMPLERATE_H__

#include "../include/samplerate.h"
#pragma comment(lib,"../libsamplerate/lib/samplerate")

#include <iostream>

namespace RL {
	namespace RecordCapture {
		//2 - 1
		static void stereo_2_mono(const int16_t *src_audio,
			int sapmples_per_channel, int16_t *dst_audio);
		//1 - 2
		static void mono_2_stereo(const int16_t *src_audio,
			int sapmples_per_channel, int16_t *dst_audio);
		//1 + 1 - 2
		static void combine_2_stereo(const int16_t *main_audio,
			const int16_t *extn_audio,
			int sapmples_per_channel, int16_t *dst_audio);
		//2 + 2 - 2
		static void combine_2_stereo2(const int16_t *main_audio,
			const int16_t *extn_audio,
			int sapmples_per_channel, int16_t *dst_audio);
		
		void get_stereo_left(const int16_t* src_audio,
			int samples_per_channel,int16_t* dst_audio);
		
		void get_stereo_right(const int16_t* src_audio,
			int samples_per_channel, int16_t* dst_audio);

		// mix 2 stero.
		static void MixerAddS16(int16_t* src1, const int16_t* src2, size_t size);

		class ReSampleRate;
		class ReSampleRateEx
		{
		public:
			ReSampleRateEx();
			~ReSampleRateEx();

			void initialization(int nSampleRateSrc, int nSampleRateDest, int nChannel);
			void resample_process(char* bufferIn, int bufferLenIn, int sample_per_channel,char* bufferOut, int &outLen);

		private:
			int m_nChannel;
			int m_nSampleRateIn;
			int m_nSampleRateOut;

			std::unique_ptr<ReSampleRate[]> pReSampleRate;
			std::unique_ptr<int16_t[]> buffer_left_src = nullptr;
			std::unique_ptr<int16_t[]> buffer_right_src = nullptr;
			std::unique_ptr<int16_t[]> buffer_left_convert = nullptr;
			std::unique_ptr<int16_t[]> buffer_right_convert = nullptr;
		};

		class ReSampleRate
		{
		public:
			ReSampleRate();
			~ReSampleRate();

			void initialization(int nSamplePerSrc, int nSamplePerDest,int nChannel);
			void resample_process(char* bufferIn, int bufferLenIn, char* bufferOut, int &outLen);

		private:

			float m_in[4906] = { 0 };
			float m_out[4096] = { 0 };
			SRC_DATA m_DataResample;
			SRC_STATE* m_DataState = nullptr;

			int m_nSampleIn;
			int m_nSampleOut;
			int m_nChannel;
		};

	}//namespace RecordCapture
}//namespace RL

#endif