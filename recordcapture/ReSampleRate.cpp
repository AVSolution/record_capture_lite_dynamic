#include "ReSampleRate.h"
#include <iostream>

namespace RL {
	namespace RecordCapture {
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

		void ReSampleRate::initialization(int nSamplePerSrc,int nSamplePerDest,int nSample ,int nChannel) 
		{
			m_nSamplesPerSec = nSamplePerDest;
			m_nChannel = nChannel;
			m_nSamples = nSample;

			m_DataResample.data_in = m_in;
			m_DataResample.input_frames = m_nSamples;
			m_DataResample.data_out = m_out;
			m_DataResample.output_frames = m_nSamples;
			m_DataResample.src_ratio = nSamplePerDest / nSamplePerSrc;
			m_DataResample.end_of_input = 1;
		}

		int ReSampleRate::resample_process(unsigned char* bufferIn,int bufferLenIn,unsigned char* bufferOut)
		{
			memset(m_in, 0, 4096 * sizeof(float));
			memset(m_out, 0, 4096 * sizeof(float));

			int RetResample = 0;
			int i = 0; int j = 0;
			for (i =0 ,j = 0; j < 4096 && j < bufferLenIn;i++,j+=m_nChannel)
			{
				m_in[i] = float(bufferIn[j]);
			}

			int bufferOutLen = 0;
			while (true) {
				src_reset(m_DataState);
				int ret = src_process(m_DataState, std::addressof(m_DataResample));
				if (0 == ret) {
					int buf_sizePcm = m_DataResample.output_frames_gen * m_nChannel;
					int i = 0; int j = 0;
					for (; i < 4096 && i < buf_sizePcm && j < m_nSamples; i += 4, j += 2)
					{
						bufferOut[j] = (unsigned char)(m_out[i]);
						bufferOut[j + 1] = (unsigned char)(m_out[i + 1]);
					}
					bufferOutLen += buf_sizePcm / 2;
				}
				else {
					printf("src_simple error: %s \n", src_strerror(ret));
					printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n",
						m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
					break;
				}

				if (m_DataResample.input_frames_used == m_nSamples) {
					m_DataResample.end_of_input = 1;
					break;
				}

				m_DataResample.data_in += m_DataResample.input_frames_used;
				m_DataResample.input_frames -= m_DataResample.input_frames_used;
				m_DataResample.end_of_input = 0;
			}

			return bufferOutLen;
		}

	}//namespace RecordCapture
}//namespace RL