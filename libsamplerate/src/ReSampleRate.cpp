#include "ReSampleRate.h"

#pragma  warning(disable:4996) 

namespace RL {
	namespace RecordCapture {

#define COMBINE(l,r) (((int32_t)(l) + (r)) >> 1)
		void stereo_2_mono(const int16_t *src_audio,
			int sapmples_per_channel, int16_t *dst_audio) {
			for (int i = 0; i < sapmples_per_channel; i++) {
				dst_audio[i] = COMBINE(src_audio[2 * i], src_audio[2 * i + 1]);
			}
		}

		void mono_2_stereo(const int16_t *src_audio,
			int sapmples_per_channel, int16_t *dst_audio) {
			for (int i = 0; i < sapmples_per_channel; i++) {
				dst_audio[2 * i] = dst_audio[2 * i + 1] = src_audio[i];
			}
		}

		void combine_2_stereo(const int16_t *main_audio,
			const int16_t *extn_audio,
			int sapmples_per_channel, int16_t *dst_audio) {
			for (int i = 0; i < sapmples_per_channel; i++) {
				// left channel
				dst_audio[2 * i] = main_audio[i];
				// right channel
				dst_audio[2 * i + 1] = extn_audio[i];
			}
		}

		void combine_2_stereo2(const int16_t *main_audio,
			const int16_t *extn_audio,
			int sapmples_per_channel, int16_t *dst_audio) {
			for (int i = 0; i < sapmples_per_channel; i++) {
				// left channel
				dst_audio[2 * i] = COMBINE(main_audio[2 * i], main_audio[2 * i + 1]);
				// right channel
				dst_audio[2 * i + 1] = COMBINE(extn_audio[2 * i], extn_audio[2 * i + 1]);
			}
		}

		void get_stereo_left(const int16_t* src_audio,
			int samples_per_channel, int16_t* dst_audio) {
			for (int i = 0; i < samples_per_channel; i++) {
				dst_audio[i] = src_audio[2 * i];
			}
		}

		void get_stereo_right(const int16_t* src_audio,
			int samples_per_channel, int16_t* dst_audio) {
			for (int i = 0; i < samples_per_channel; i++) {
				dst_audio[i] = src_audio[2 * i + 1];
			}
		}

		//class ResampleRateEx
		ReSampleRateEx::ReSampleRateEx()
		{
		}

		ReSampleRateEx::~ReSampleRateEx()
		{
		}

		void ReSampleRateEx::initialization(int nSampleRateSrc, int nSampleRateDest, int nChannel)
		{
			m_nSampleRateIn = nSampleRateSrc;
			m_nSampleRateOut = nSampleRateDest;
			m_nChannel = nChannel;
		}

		void ReSampleRateEx::resample_process(char* bufferIn, int bufferLenIn, int sample_per_channel, char* bufferOut, int &outLen)
		{
			if (pReSampleRate == nullptr)
				pReSampleRate = std::make_unique<ReSampleRate[]>(m_nChannel);

			if (pReSampleRate) {
				for (int nChannel = 0; nChannel < m_nChannel; nChannel++) {
					pReSampleRate[nChannel].initialization(m_nSampleRateIn, m_nSampleRateOut, 1);
				}
			}

			if (buffer_left_src == nullptr) {
				buffer_left_src = std::make_unique<int16_t[]>(m_nSampleRateIn / 100);
				memset(buffer_left_src.get(), 0, m_nSampleRateIn / 100 * sizeof(int16_t));
			}

			if (buffer_right_src == nullptr) {
				buffer_right_src = std::make_unique<int16_t[]>(m_nSampleRateIn / 100);
				memset(buffer_right_src.get(), 0, m_nSampleRateIn / 100 * sizeof(int16_t));
			}

			if (buffer_left_convert == nullptr) {
				buffer_left_convert = std::make_unique<int16_t[]>(m_nSampleRateOut / 100);
				memset(buffer_left_convert.get(), 0, m_nSampleRateOut / 100 * sizeof(int16_t));
			}

			if (buffer_right_convert == nullptr) {
				buffer_right_convert = std::make_unique<int16_t[]>(m_nSampleRateOut / 100);
				memset(buffer_right_convert.get(), 0, m_nSampleRateOut / 100 * sizeof(int16_t));
			}

			if (m_nChannel == 1) {
				pReSampleRate[0].resample_process(bufferIn, bufferLenIn, bufferOut, outLen);
			}
			else if(m_nChannel == 2){
				int outLen1 = 0;

				get_stereo_left((int16_t*)bufferIn, sample_per_channel, buffer_left_src.get());
				pReSampleRate[0].resample_process((char*)buffer_left_src.get(), bufferLenIn / 2, (char*)buffer_left_convert.get(), outLen1);

				get_stereo_right((int16_t*)bufferIn, sample_per_channel, buffer_right_src.get());
				pReSampleRate[1].resample_process((char*)buffer_right_src.get(), bufferLenIn / 2, (char*)buffer_right_convert.get(), outLen1);

				combine_2_stereo(buffer_left_convert.get(), buffer_right_convert.get(), sample_per_channel, (int16_t*)bufferOut);
				outLen = outLen1 * 2;
			}
		}

		void ReSampleRateEx::resample_process_fixed(char* bufferIn, int bufferLenIn, int sample_per_channel, unsigned char* bufferOut, int &outLen)
		{
			if (pReSampleRate == nullptr)
				pReSampleRate = std::make_unique<ReSampleRate[]>(m_nChannel);

			if (pReSampleRate) {
				for (int nChannel = 0; nChannel < m_nChannel; nChannel++) {
					pReSampleRate[nChannel].initialization(m_nSampleRateIn, m_nSampleRateOut, 1);
				}
			}

			if (buffer_left_src == nullptr) {
				buffer_left_src = std::make_unique<int16_t[]>(sample_per_channel);
				memset(buffer_left_src.get(), 0, sample_per_channel * sizeof(int16_t));
			}

			if (buffer_right_src == nullptr) {
				buffer_right_src = std::make_unique<int16_t[]>(sample_per_channel);
				memset(buffer_right_src.get(), 0, sample_per_channel * sizeof(int16_t));
			}

			if (buffer_left_convert == nullptr) {
				buffer_left_convert = std::make_unique<int16_t[]>(sample_per_channel);
				memset(buffer_left_convert.get(), 0, sample_per_channel * sizeof(int16_t));
			}

			if (buffer_right_convert == nullptr) {
				buffer_right_convert = std::make_unique<int16_t[]>(sample_per_channel);
				memset(buffer_right_convert.get(), 0, sample_per_channel * sizeof(int16_t));
			}

			if (m_nChannel == 1) {
				pReSampleRate[0].resample_process_fixed(bufferIn, bufferLenIn,sample_per_channel, bufferOut, outLen);
			}
			else if (m_nChannel == 2) {
				int outLen1 = 0;

				get_stereo_left((int16_t*)bufferIn, sample_per_channel, buffer_left_src.get());
				pReSampleRate[0].resample_process_fixed((char*)buffer_left_src.get(), bufferLenIn / 2,sample_per_channel, (unsigned char*)buffer_left_convert.get(), outLen1);

				//static FILE* fLeft = nullptr;
				//if (fLeft == nullptr) {
				//	fLeft = fopen("left_src.pcm", "wb+");
				//}
				//fwrite(buffer_left_src.get(), sample_per_channel * 2, 1, fLeft);

				get_stereo_right((int16_t*)bufferIn, sample_per_channel, buffer_right_src.get());
				pReSampleRate[1].resample_process_fixed((char*)buffer_right_src.get(), bufferLenIn / 2, sample_per_channel, (unsigned char*)buffer_right_convert.get(), outLen1);

				//static FILE* fRight = nullptr;
				//if (fRight == nullptr) {
				//	fRight = fopen("right_src.pcm", "wb+");
				//}
				//fwrite(buffer_right_src.get(), sample_per_channel * 2, 1, fRight);

				combine_2_stereo2(buffer_left_convert.get(), buffer_right_convert.get(), outLen1 / 2, (int16_t*)bufferOut);
				
				outLen = outLen1 * 2;
				//outLen = 0;
			}
		}


		//class ResampleRate
		ReSampleRate::ReSampleRate() {
			int error = 0;
			m_DataState = src_new(SRC_LINEAR, 2, std::addressof(error));
			if (!m_DataState) {
				printf("\n\nError : src_new() failed : %s.\n\n", src_strerror(error));
			}
		}

		ReSampleRate::~ReSampleRate() {
			src_delete(m_DataState);
		}

		void ReSampleRate::initialization(int nSamplePerSrc,int nSamplePerDest,int nChannel) 
		{
			m_nChannel = nChannel;
			m_nSampleIn = nSamplePerSrc / 100.0 * 2;//default support 16bit.
			m_nSampleOut = nSamplePerDest / 100.0 * 2;
		}

		void ReSampleRate::resample_process(char* bufferIn,int bufferLenIn, char* bufferOut,int &outLen)
		{
			//reset buffer.
			memset(m_in, 0, 4096 * sizeof(float));
			memset(m_out, 0, 4096 * sizeof(float));
	
			int i = 0;
			for (;i < 4096 && i < bufferLenIn;i++)
			{
				m_in[i] = bufferIn[i];
			}

			m_DataResample.end_of_input = 1;
			m_DataResample.data_in = m_in;
			m_DataResample.input_frames = m_nSampleIn;
			m_DataResample.data_out = m_out;
			m_DataResample.output_frames = m_nSampleOut;
			m_DataResample.src_ratio = 1.0 * m_nSampleOut / m_nSampleIn;

			while (true) {
				src_reset(m_DataState);
				int ret = src_process(m_DataState, std::addressof(m_DataResample));
				if (0 == ret) {
					int buf_sizePCM = m_DataResample.output_frames_gen * m_nChannel;
					int i = 0; int j = 0;
					for (; i < 4096 && i < buf_sizePCM && j < buf_sizePCM; i ++,j ++)
					{
						bufferOut[j] = (unsigned char)(m_out[i]);
						//bufferOut[j + 1] = (unsigned char)(m_out[i + 1]);
						//bufferOut[j + 2] = (unsigned char)(m_out[i + 2]);
						//bufferOut[j + 3] = (unsigned char)(m_out[i + 3]);
					}
					buf_sizePCM = buf_sizePCM;
					outLen = buf_sizePCM;

					//printf("-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d] src_ratio[%f]------ \n",
					//	m_DataResample.output_frames_gen, m_DataResample.input_frames_used,
					//	m_DataResample.end_of_input, m_DataResample.src_ratio);
				}
				else {
					printf("src_simple error: %s \n", src_strerror(ret));
					printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n",
						m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
					break;
				}

				if (m_DataResample.input_frames_used == m_nSampleIn) {
					m_DataResample.end_of_input = 1;
					break;
				}

				m_DataResample.data_in += m_DataResample.input_frames_used;
				m_DataResample.input_frames -= m_DataResample.input_frames_used;
				m_DataResample.end_of_input = 0;
			}
		}

		void ReSampleRate::resample_process_fixed(char* bufferIn, int bufferLenIn, int sample_per_channel, unsigned char* bufferOut, int &outLen)
		{
			//(bufferLenIn == sample_per_channel * sizeof(16) * m_nChannel); forever true.

			//reset buffer.
			memset(m_in, 0, 4096 * sizeof(float));
			memset(m_out, 0, 4096 * sizeof(float));

			int i = 0;
			for (; i < 4096 && i < bufferLenIn; i++)
			{
				m_in[i] = bufferIn[i];
			}

			m_DataResample.end_of_input = 1;
			m_DataResample.data_in = m_in;
			m_DataResample.input_frames = sample_per_channel * 2;
			m_DataResample.data_out = m_out;
			m_DataResample.src_ratio = 1.0 * m_nSampleOut / m_nSampleIn;
			m_DataResample.output_frames = sample_per_channel * 2 * m_nSampleOut / m_nSampleIn;

			int nCountSample = 0;
			outLen = 0;
			while (true) {
				src_reset(m_DataState);
				int ret = src_process(m_DataState, std::addressof(m_DataResample));
				if (0 == ret) {
					int buf_sizePCM = m_DataResample.output_frames_gen * m_nChannel;
					int i = 0; int j = 0;
					for (; i < 4096 && i < buf_sizePCM && j < buf_sizePCM; i++, j++)
					{
						bufferOut[j + nCountSample] = (unsigned char)(m_out[i]);
					}
					buf_sizePCM = buf_sizePCM;
					outLen += buf_sizePCM;

					printf("-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d] src_ratio[%f]------ \n",
						m_DataResample.output_frames_gen, m_DataResample.input_frames_used,
						m_DataResample.end_of_input, m_DataResample.src_ratio);
				}
				else {
					printf("src_simple error: %s \n", src_strerror(ret));
					printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n",
						m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
					break;
				}

				if (m_DataResample.input_frames_used == m_DataResample.input_frames ||
					m_DataResample.output_frames_gen == m_DataResample.output_frames) {
					m_DataResample.end_of_input = 1;
					break;
				}
				nCountSample += m_DataResample.input_frames_used;
				m_DataResample.data_in += m_DataResample.input_frames_used;
				m_DataResample.input_frames -= m_DataResample.input_frames_used;
				m_DataResample.end_of_input = 0;
			}
		}

	}//namespace RecordCapture
}//namespace RL