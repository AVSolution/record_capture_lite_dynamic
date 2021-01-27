#ifndef __RESAMPLERATE_H__
#define __RESAMPLERATE_H__

#include "libsamplerate/include/common.h"
#include "libsamplerate/include/samplerate.h"
#pragma comment(lib,"libsamplerate/samplerate")

namespace RL {
	namespace RecordCapture {

		static int samplerate_process_output(
			unsigned char* frameSrc, int lenSrc,
			int nSamplePerSecSrc, int nChannelSrc, int nBytePerSampleSrc,
			unsigned char* frameDest, int lenDest,
			int nSamplePerSec, int nChannel, int nBytePerSample)
		{
			SRC_DATA m_DataResample;
			float in[4096] = { 0 };
			float out[4096] = { 0 };

			int samples = 2048;
			int channel = 2;

			m_DataResample.data_in = in;
			m_DataResample.input_frames = samples;
			m_DataResample.data_out = out;
			m_DataResample.output_frames = samples;
			m_DataResample.src_ratio = nSamplePerSec / nSamplePerSecSrc;


			return 0;
		}

		class ReSampleRate
		{
		public:
			ReSampleRate();
			~ReSampleRate();

			void initialization(int nSamplePerSrc, int nSamplePerDest, int nSample,int nChannel);
			int resample_process(unsigned char* bufferIn, int bufferLenIn, unsigned char* bufferOut);

		private:

			float m_in[4906] = { 0 };
			float m_out[4096] = { 0 };
			SRC_DATA m_DataResample;
			SRC_STATE* m_DataState = nullptr;

			int m_nSamplesPerSec;
			int m_nChannel;
			int m_nBytePerSample;
			int m_nSamples;
			int m_buffer_size;
		};

	}//namespace RecordCapture
}//namespace RL

#endif