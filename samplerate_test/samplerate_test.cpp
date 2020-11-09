// samplerate_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../libsamplerate/src/ReSampleRate.h"

#include <iostream>
#pragma  warning(disable:4996)
#include <thread>

#include "../speed_resample/AudioChunk.h"
#include "../speed_resample/SpeexResampler.h"

#include "../speed_resample/ResamplerEx.h"
#pragma comment(lib,"..\\bind\\resample.lib")//debug mode.

#include "../my_speed_resampler/include/speex/speex_resampler.h"
#pragma comment(lib,"../my_speed_resampler/lib/libspeexdsp")

//#define SAMPLERATE
#define SAMPLERATEX
//#define SPEED_SAMPLERATE
//#define SPEED_SAMPLERATEEx
//#define mian_test
//#define my_speed_resample

int main()
{
	int sampleIn = 44100;
	int samleout = 16000;
	int nChannel = 1;
	int nRead_Buffer = sampleIn / 100 * 2 * nChannel;//the length to read from source pcm file.
	int nConvert_Buffer = samleout / 100 * 2 * nChannel;

	int bufferLen_max = 4096;
	std::unique_ptr<char[]> pReadBuffer = std::make_unique<char[]>(bufferLen_max);
	memset(pReadBuffer.get(), 0, bufferLen_max);

	std::unique_ptr<char[]> pConvertBuffer = std::make_unique<char[]>(bufferLen_max);
	memset(pConvertBuffer.get(), 0, bufferLen_max);

	FILE* fPcmSrc = fopen("44100_1_16.pcm", "rb+");
	if (nullptr == fPcmSrc) {
		std::cout << "" << std::endl;
		return -1;
	}

#ifdef SAMPLERATE
	//initialization the samplerate param list.
	RL::RecordCapture::ReSampleRate samplerate;
	samplerate.initialization(sampleIn, samleout, nChannel);

	FILE* fPcmDst = fopen("speaker_samplerate_convert_1.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}

	int nFrame = 0;
	getchar();
	while ((fread(pReadBuffer.get(), nRead_Buffer , 1, fPcmSrc) != 0)) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;

		memset(pConvertBuffer.get(), 0, 2048);
		int outLen = 0;
		samplerate.resample_process_fixed(pReadBuffer.get(), nRead_Buffer ,nRead_Buffer / sizeof(int16_t)/nChannel, pConvertBuffer.get(),outLen);
		if (outLen) {
			fwrite(pConvertBuffer.get(), outLen, 1, fPcmDst);
			fflush(fPcmDst);
		}
		//nRead_Buffer += sizeof(int16_t) * nChannel * 2 * 2;
		//if (nRead_Buffer > 2048)
		//	nRead_Buffer = 480 * 2 * 2;
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
#endif

#ifdef SAMPLERATEX
	int nFrame = 0;
	getchar();

	//FILE* fPcmSrc_left = fopen("speaker_left.pcm", "wb+");
	//if (nullptr == fPcmSrc_left) {
	//	std::cout << "" << std::endl;
	//	return -1;
	//}
	//FILE* fPcmSrc_right = fopen("speaker_right.pcm", "wb+");
	//if (nullptr == fPcmSrc_right) {
	//	std::cout << "" << std::endl;
	//	return -1;
	//}

	//FILE* fPcmSrc_left_1 = fopen("speaker_left_1.pcm", "wb+");
	//if (nullptr == fPcmSrc_left_1) {
	//	std::cout << "" << std::endl;
	//	return -1;
	//}

	FILE* fPcmDst = fopen("speaker_samplerateex_convert_1.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}
/*
	std::unique_ptr<char[]> left_src = std::make_unique<char[]>(nRead_Buffer / 2);
	std::unique_ptr<char[]> right_src = std::make_unique<char[]>(nRead_Buffer / 2);
	std::unique_ptr<int16_t[]> right_src_1 = std::make_unique<int16_t[]>(nRead_Buffer / 2 / 2);*/

	nRead_Buffer *= 2;
	RL::RecordCapture::ReSampleRateEx sampleEx;
	sampleEx.initialization(sampleIn, samleout, nChannel);
	while ((fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0)) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;
		
		int outLen = 0;
		sampleEx.resample_process_fixed(pReadBuffer.get(), nRead_Buffer, nRead_Buffer / sizeof(int16_t) / nChannel, pConvertBuffer.get(), outLen);
		if (outLen) {
			fwrite(pConvertBuffer.get(), outLen, 1, fPcmDst);
			fflush(fPcmDst);
		}

		//for (int i = 0; i < nRead_Buffer / 4; i++) {
		//	int16_t ld = *(int16_t*)(pReadBuffer.get() + i * 4);
		//	int16_t rd = *(int16_t*)(pReadBuffer.get() + i * 4 + 2);

		//	fwrite(&ld, sizeof(int16_t), 1, fPcmSrc_left);
		//	fflush(fPcmSrc_left);

		//	fwrite(&rd, sizeof(int16_t), 1, fPcmSrc_right);
		//	fflush(fPcmSrc_right);
		//}
		
		//RL::RecordCapture::get_stereo_left((int16_t*)pReadBuffer.get(), 480, (int16_t*)right_src_1.get());
		//fwrite(left_src.get(), sizeof(int16_t),480, fPcmSrc_left_1);
		//fflush(fPcmSrc_left_1);

		//RL::RecordCapture::get_stereo_left((int16_t*)pReadBuffer.get(), 480, (int16_t*)left_src.get());
		//fwrite(left_src.get(), nRead_Buffer / 2, 1, fPcmSrc_left);
		//fflush(fPcmSrc_left);

		//RL::RecordCapture::get_stereo_right((int16_t*)pReadBuffer.get(), 480, (int16_t*)right_src.get());
		//fwrite(right_src.get(), nRead_Buffer / 2, 1, fPcmSrc_right);
		//fflush(fPcmSrc_right);	
	}
#endif

#ifdef  SPEED_SAMPLERATE

	int nFrame = 0;
	getchar();

	FILE* fPcmDst = fopen("speaker_speed_convert.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}

	CAudioChunk audioChunk;
	CAudioChunk audioChunkOut;
	audioChunk.Reset();
	IResampler* pResampler = nullptr;
	while (fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;
		audioChunk.SetData(pReadBuffer.get(), nRead_Buffer, 48000, 2, 16, false);
		if (pResampler == nullptr) {
			pResampler = CreateResampler();
			pResampler->Init(2, 48000, 44100);
		}

		if (pResampler) {
			int outsize = pResampler->Process(std::addressof(audioChunk), std::addressof(audioChunkOut));
			if (outsize) {
				fwrite(audioChunkOut.GetData(), outsize, 1, fPcmDst);
				fflush(fPcmDst);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

#endif

#ifdef  SPEED_SAMPLERATEEx

	std::cout << sizeof(float);
	int nFrame = 0;
	getchar();

	FILE* fPcmDst = fopen("speaker_speedex_convert.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}

	CAudioChunk audioChunk;
	CAudioChunk audioChunkOut;
	audioChunk.Reset();
	IResamplerEx* pResamplerEx = nullptr;
	while (fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0) {
		DWORD time_start = GetTickCount();
		if (nFrame % 20 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;
		audioChunk.SetData(pReadBuffer.get(), nRead_Buffer, 48000, 2, 16, false);
		if (pResamplerEx == nullptr) {
			pResamplerEx = CreateResamplerEx();
			pResamplerEx->Init(2, 48000, 44100,2048);
		}

		if (pResamplerEx) {
			int outsize = pResamplerEx->Process(std::addressof(audioChunk), std::addressof(audioChunkOut));
			if (outsize) {
				fwrite(audioChunkOut.GetData(), outsize, 1, fPcmDst);
				fflush(fPcmDst);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//std::cout << "interval: " << GetTickCount() - time_start << std::endl;
	}

#endif

#ifdef my_speed_resample
	SpeexResamplerState *resampler = NULL;
	int err = 0;

	resampler = speex_resampler_init(2, 48000, 44100, 10, &err);
	speex_resampler_set_rate(resampler, 48000, 44100);
	speex_resampler_skip_zeros(resampler);

	FILE* pcmfd = fopen("speaker.pcm", "rb+");
	if (!pcmfd)
	{
		//cout << "open test.cpm faile" << endl;
		printf("open test.cpm faile \n");
		return -1;
	}


	FILE* outfd = fopen("44100k.pcm", "wb+");
	if (!outfd)
	{
		fclose(pcmfd);
		printf("open test.cpm faile \n");
		return -1;
	}

	int samples = 480 * 2; 
	int channel = 2;

	int buffer_size = samples * channel;
	char* pcmbuffer = new char[buffer_size];
	memset(pcmbuffer, 0, buffer_size);

	unsigned char* out_buffer = new unsigned char[buffer_size * 2];
	memset(out_buffer, 0, buffer_size * 2);

	unsigned char* last_buffer = new unsigned char[buffer_size];
	memset(last_buffer, 0, buffer_size);

	int frame = 0;
	int ret = 0;

	while (fread(pcmbuffer, buffer_size, 1, pcmfd) != 0)
	{
		memset(out_buffer, 0, buffer_size * 2);
		memset(last_buffer, 0, buffer_size);

		unsigned int inlen = buffer_size / 2;
		unsigned int outlen = buffer_size;	

		ret = speex_resampler_process_interleaved_int(resampler, (short*)pcmbuffer, &inlen, (short*)out_buffer, &outlen);
		if (ret == RESAMPLER_ERR_SUCCESS)
		{
			//int i = 0, j = 0;
			//for (; i < outlen * 2; i += 4, j += 2)
			//{
			//	last_buffer[j] = out_buffer[i];
			//	last_buffer[j + 1] = out_buffer[i + 1];
			//}
			fwrite(out_buffer, sizeof(short), outlen, outfd);  /* 输出双声道 */
			//fwrite(last_buffer, sizeof(short), outlen / 2, outfd);


			printf("processed[%d] outlen = %d \n", inlen, outlen);
		}
		else
		{
			printf("error: %d\n", ret);
		}
	}


	speex_resampler_destroy(resampler);

#endif

#ifdef mian_test

	SRC_DATA m_DataResample;

	SRC_STATE* src_state = NULL;
	int error = -1;
	src_state = src_new(SRC_LINEAR, 2, &error);
	if (!src_state)
	{
		printf("\n\nError : src_new() failed : %s.\n\n", src_strerror(error));
		return -1;
	}

	float in[4096] = { 0 };
	float out[4096] = { 0 };

	int samples = 480 * 2;
	int sampleout = 441 * 2;
	int channel = 2;

	m_DataResample.data_in = in;
	m_DataResample.input_frames = samples;
	m_DataResample.data_out = out;
	m_DataResample.output_frames = sampleout;
	m_DataResample.src_ratio = 1.0 * sampleout / samples;  /* 输出采样率/输入采样率 */


	int RetResample = 0;

	FILE* pcmfd = fopen("speaker.pcm", "rb+");
	if (!pcmfd)
	{
		//cout << "open test.cpm faile" << endl;
		printf("open test.cpm faile \n");
		return -1;
	}

	FILE* outfd_process = fopen("speaker_samplerate_convert.pcm", "wb+");
	if (!outfd_process)
	{
		printf("open 8k_process.cpm faile \n");
		fclose(pcmfd);
		return -1;
	}

	int buffer_size = samples * channel;
	char* pcmbuffer = new char[buffer_size];
	memset(pcmbuffer, 0, buffer_size);

	unsigned char* out_buffer = new unsigned char[buffer_size];
	memset(out_buffer, 0, buffer_size);

	int frame = 0;
	while (fread(pcmbuffer, buffer_size, 1, pcmfd) != 0)
	{
		if (frame % 10 == 0)
			printf("processing the %d frame \n", frame);

		memcpy(out_buffer, pcmbuffer, buffer_size);

		memset(in, 0, 4096 * sizeof(float));
		memset(out, 0, 4096 * sizeof(float));

		for (int j = 0; j < 4096 && j < buffer_size; j++)
		{
			in[j] = pcmbuffer[j];
		}

		/* SRC_PROCESS 处理流程 */
		m_DataResample.end_of_input = 1;
		m_DataResample.data_in = in;
		m_DataResample.input_frames = samples;
		m_DataResample.data_out = out;
		m_DataResample.output_frames = sampleout;
		m_DataResample.src_ratio = 1.0 * sampleout / samples;  /* 输出采样率/输入采样率 */

		while (1)
		{
			src_reset(src_state);
			int ret = src_process(src_state, &m_DataResample);
			if (0 == ret)
			{
				int buf_sizePCM = m_DataResample.output_frames_gen * channel;
				int i = 0, j = 0;
				for (; i < 4096 && i < buf_sizePCM && j < buf_sizePCM; i += 4, j += 4)
				{
					out_buffer[j] = (unsigned char)(out[i]);
					out_buffer[j + 1] = (unsigned char)(out[i + 1]);
					out_buffer[j + 2] = (unsigned char)(out[i + 2]);
					out_buffer[j + 3] = (unsigned char)(out[i + 3]);
				}
				buf_sizePCM = buf_sizePCM / 2;

				fwrite(out_buffer, 1, buf_sizePCM, outfd_process);
				fflush(outfd_process);

				memset(out_buffer, 0, buffer_size);
				memset(out, 0, 4096 * sizeof(float));
				printf("-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d] src_ratio[%f]------ \n",
					m_DataResample.output_frames_gen, m_DataResample.input_frames_used,
					m_DataResample.end_of_input, m_DataResample.src_ratio);
			}
			else
			{
				printf("src_simple error: %s \n", src_strerror(ret));
				printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n",
					m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
				break;
			}

			if (m_DataResample.input_frames_used == samples)
			{
				m_DataResample.end_of_input = 1;
				break;
			}

			m_DataResample.data_in += m_DataResample.input_frames_used;
			m_DataResample.input_frames -= m_DataResample.input_frames_used;
			m_DataResample.output_frames_gen = 0;

		}

		frame++;
		memset(out_buffer, 0, buffer_size);
		memset(pcmbuffer, 0, buffer_size);
	}

	delete[] pcmbuffer;
	src_delete(src_state);
	fclose(pcmfd);
	fclose(outfd_process);
#endif

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
