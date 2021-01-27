// samplerate_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../libsamplerate/src/ReSampleRate.h"// support unity function.

#include <iostream>
#pragma  warning(disable:4996)
#include <thread>

//#define SAMPLERATEX(deprecated)
//#define SAMPLERATE_NEW(deprecated)
//#define SAMPLERATE//存在逻辑问题
//#define SPEEX_DSP//性能更好些,速度更快
#define RESAMPLE

#ifdef SPEED_SAMPLERATEEX//base hook resample
#include "../speed_resample/ResamplerEx.h"
#pragma comment(lib,"..\\bind\\resample.lib")//debug mode.
#endif

#ifdef SPEEX_DSP//speex.dsp lib
#include "../speex_resampler/include/speex/speex_resampler.h"
#pragma comment(lib,"../speex_resampler/lib/libspeexdsp")
#endif

#ifdef  RESAMPLE
#include "resampler.h"
#endif

int main()
{
	//read buffer and  convert buffer should enough ; read len and convert len should releated.
	int sampleIn = 8000;
	int sampleout = 48000;
	int nChannel = 2;
	int nRead_Buffer = sampleIn / 100 * 2 * nChannel;//the length to read from source pcm file.
	int nConvert_Buffer = sampleout / 100 * 2 * nChannel;

	int bufferLen_max = 4096;
	std::unique_ptr<char[]> pReadBuffer = std::make_unique<char[]>(bufferLen_max);
	memset(pReadBuffer.get(), 0, bufferLen_max);

	std::unique_ptr<unsigned char[]> pConvertBuffer = std::make_unique<unsigned char[]>(bufferLen_max);
	memset(pConvertBuffer.get(), 0, bufferLen_max);

	FILE* fPcmSrc = fopen("pcm8k.pcm", "rb+");
	if (nullptr == fPcmSrc) {
		std::cout << "" << std::endl;
		return -1;
	}

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
	sampleEx.initialization(sampleIn, sampleout, nChannel);
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

#ifdef SAMPLERATE_NEW
	int nFrame = 0;
	getchar();
	FILE* fPcmDst = fopen("speaker_samplerate_new_convert.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}

	//nRead_Buffer *= 2;
	RL::RecordCapture::ReSampleRateNew samplenew;
	samplenew.initialization(sampleIn, sampleout, nChannel);
	while ((fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0)) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;

		size_t outLen = 0;
		samplenew.resample_process_fixed((int16_t*)pReadBuffer.get(), nRead_Buffer / sizeof(int16_t), nRead_Buffer / sizeof(int16_t) / nChannel, (int16_t*)pConvertBuffer.get(), outLen);
		if (outLen) {
			fwrite(pConvertBuffer.get(), outLen, sizeof(int16_t), fPcmDst);
			fflush(fPcmDst);
		}
	}
#endif

#ifdef SPEEX_DSP
	getchar();
	auto starttps = std::chrono::high_resolution_clock::now();

	SpeexResamplerState *resampler = NULL;
	int err = 0;

	resampler = speex_resampler_init(nChannel,sampleIn, sampleout, 10, &err);
	speex_resampler_set_rate(resampler, sampleIn, sampleout);
	speex_resampler_skip_zeros(resampler);

	FILE* outfd = fopen("speex_samplerate.pcm", "wb+");
	if (!outfd)
	{
		fclose(outfd);
		printf("open speed_samplerate.cpm faile \n");
		return -1;
	}

	int nFrame = 0;
	int ret = 0;
	while (fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0)
	{
		if (nFrame % 20 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;

		spx_uint32_t out_len = nConvert_Buffer / sizeof(int16_t);
		spx_uint32_t in_len = nRead_Buffer / sizeof(int16_t);

		if(nChannel == 1)
			ret = speex_resampler_process_interleaved_int(resampler,(int16_t*)pReadBuffer.get(),&in_len,	(int16_t*)pConvertBuffer.get(),&out_len);
		else if (nChannel == 2)
			ret = speex_resampler_process_int(resampler, 0, (int16_t*)pReadBuffer.get(), &in_len, (int16_t*)pConvertBuffer.get(), &out_len);
		if (ret == RESAMPLER_ERR_SUCCESS)
		{
			fwrite(pConvertBuffer.get(), sizeof(int16_t), out_len, outfd);
			//printf("processed in_len: [%d]; out_len = %d \n", in_len, out_len);
		}
		else
		{
			printf("error: %d\n", ret);
		}
	}

	speex_resampler_destroy(resampler);

	printf("speex_dsp all time count: %f", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - starttps).count() / 1000.0);

#endif

#ifdef RESAMPLE
	int nFrame = 0;
	getchar();
	auto starttps = std::chrono::high_resolution_clock::now();

	FILE* outfd = fopen("resample.pcm", "wb+");
	if (!outfd)
	{
		fclose(outfd);
		printf("open resample.cpm faile \n");
		return -1;
	}

	int SrcLen = nRead_Buffer / sizeof(short);
	std::unique_ptr<float []> destBuffer = std::make_unique<float[]>(SrcLen);

	musly::resampler resampler(sampleIn, sampleout);
	while (fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;
		short *pSrcBuffer = (short*)pReadBuffer.get();
		src_short_to_float_array(pSrcBuffer,destBuffer.get(),SrcLen);
		std::vector<float> vecSample = resampler.resample(destBuffer.get(),SrcLen);
		src_float_to_short_array(vecSample.data(), (short*)pConvertBuffer.get(), vecSample.size());
		fwrite((void*)pConvertBuffer.get(), 2, vecSample.size(), outfd);
	}

	printf("resample all time count: %f", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - starttps).count() / 1000.0);

#endif

#ifdef SAMPLERATE
	getchar();
	auto starttps = std::chrono::high_resolution_clock::now();

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
	memset(in, 0, 4096 * sizeof(float));
	memset(out, 0, 4096 * sizeof(float));

	FILE* outfd = fopen("samplerate.pcm", "wb+");
	if (!outfd)
	{
		printf("open samplerate.cpm faile \n");
		fclose(outfd);
		return -1;
	}

	int frame = 0;
	sampleIn = sampleIn / 100;
	sampleout = sampleout / 100;

	m_DataResample.data_in = in;
	m_DataResample.input_frames = sampleIn;
	m_DataResample.data_out = out;
	m_DataResample.output_frames = sampleout + 10;
	m_DataResample.src_ratio = 1.0 * sampleout / sampleIn;  /* 输出采样率/输入采样率 */

	while (fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0)
	{
		if (frame % 10 == 0)
			printf("processing the %d frame \n", frame);
		frame++;
		
		short*pSrcBuffer = (short*)pReadBuffer.get();
		int pSrcBuferLen = nRead_Buffer / sizeof(short);
		src_short_to_float_array(pSrcBuffer,in,pSrcBuferLen);

		while (true)
		{
			src_reset(src_state);
			int ret = src_process(src_state, &m_DataResample);
			if (0 == ret)
			{
				src_float_to_short_array(out, (short*)pConvertBuffer.get(), m_DataResample.output_frames_gen * nChannel);
				fwrite(pConvertBuffer.get(), sizeof(short), m_DataResample.output_frames_gen * nChannel, outfd);
				//printf("-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d] src_ratio[%f]------ \n",
					//m_DataResample.output_frames_gen, m_DataResample.input_frames_used,
					//m_DataResample.end_of_input, m_DataResample.src_ratio);
			}
			else
			{
				printf("src_simple error: %s \n", src_strerror(ret));
				printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n",
					m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
				break;
			}

			if (m_DataResample.input_frames_used == sampleIn || m_DataResample.output_frames_gen == sampleout)
			{
				m_DataResample.end_of_input = 1;
				break;
			}

			m_DataResample.data_in += m_DataResample.input_frames_used;
			m_DataResample.input_frames -= m_DataResample.input_frames_used;
			m_DataResample.output_frames_gen = 0;

		}
	}
	src_delete(src_state);

	printf("samplerate all time count: %f", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - starttps).count() / 1000.0);
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
