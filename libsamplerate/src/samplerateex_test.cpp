// samplerateex_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "../libsamplerate/src/ReSampleRate.h"
#pragma  warning(disable:4996)
#include <thread>

int main()
{
	int sampleIn = 44100;
	int samleout = 16000;
	int nChannel = 2;
	int nRead_Buffer = sampleIn / 100 * 2 * nChannel;//the length to read from source pcm file.
	int nConvert_Buffer = samleout / 100 * 2 * nChannel;

	std::unique_ptr<char[]> pReadBuffer = std::make_unique<char[]>(nRead_Buffer);
	memset(pReadBuffer.get(), 0, nRead_Buffer);

	std::unique_ptr<char[]> pConvertBuffer = std::make_unique<char[]>(nConvert_Buffer);
	memset(pConvertBuffer.get(), 0, nConvert_Buffer);

	FILE* fPcmSrc = fopen("44100_2_16.pcm", "rb+");
	if (nullptr == fPcmSrc) {
		std::cout << "" << std::endl;
		return -1;
	}

	int nFrame = 0;

	FILE* fPcmDst = fopen("speaker_samplerateex_convert.pcm", "wb+");
	if (nullptr == fPcmDst) {
		std::cout << "" << std::endl;
		return -1;
	}

	std::unique_ptr<char[]> left_src = std::make_unique<char[]>(nRead_Buffer / 2);
	std::unique_ptr<char[]> right_src = std::make_unique<char[]>(nRead_Buffer / 2);
	std::unique_ptr<int16_t[]> right_src_1 = std::make_unique<int16_t[]>(nRead_Buffer / 2 / 2);

	RL::RecordCapture::ReSampleRateEx sampleEx;
	sampleEx.initialization(sampleIn, samleout, nChannel);
	while ((fread(pReadBuffer.get(), nRead_Buffer, 1, fPcmSrc) != 0)) {
		if (nFrame % 10 == 0)
			printf("processing the %d frame \n", nFrame);
		nFrame++;

		int outLen = 0;
		sampleEx.resample_process(pReadBuffer.get(), nRead_Buffer, sampleIn / 100, pConvertBuffer.get(), outLen);
		if (outLen) {
			fwrite(pConvertBuffer.get(), outLen, 1, fPcmDst);
			fflush(fPcmDst);
		}
	}

    std::cout << "Hello World!\n";
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
