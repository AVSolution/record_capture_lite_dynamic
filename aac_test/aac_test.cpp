// aac_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "faacImpl.h"
#include <chrono>

int main()
{
	getchar();

	auto startpts = std::chrono::high_resolution_clock::now();

	faacImpl faacImplInstance;
	char* id, *copyright;
	faacImpl::getVersion(&id, &copyright);
	std::cout << "id: " << id << "copyright" << copyright;

	int nSample = 44100;
	int nChannel = 2;
	int bytePerSample = 2; 
	unsigned long  inputSamples = 0;
	unsigned long maxOutputBytes = 0;

	//setformat
	faacImplInstance.initEncoder(nSample, 2, &inputSamples, &maxOutputBytes);
	faacEncConfigurationPtr pConfig = faacImplInstance.getEncConfigration();
	pConfig->inputFormat = FAAC_INPUT_16BIT;
	pConfig->outputFormat = 1;
	pConfig->aacObjectType = LOW;
	pConfig->bitRate = 48000;
	pConfig->bandWidth = 32000;
	faacImplInstance.setEncConfigituration(pConfig);
	//allocate buffer 
	int inputBuffer = inputSamples * bytePerSample;
	std::unique_ptr<uint8_t[]> srcBuffer = std::make_unique<uint8_t[]>(inputBuffer);
	memset(srcBuffer.get(), 0, inputBuffer);
	std::unique_ptr<uint8_t[]> destBuffer = std::make_unique<uint8_t[]>(maxOutputBytes);
	memset(destBuffer.get(), 0, maxOutputBytes);

	FILE* pFileSrc = fopen("speed_samplerate_out_44100.pcm", "rb");
	FILE* pFileDest = fopen("mytest.aac", "wb");

	while (fread(srcBuffer.get(), 1, inputBuffer, pFileSrc) != 0) {

		int writeen = faacImplInstance.aacEncode((int32_t*)srcBuffer.get(), inputSamples,
			destBuffer.get(), maxOutputBytes);

		fwrite(destBuffer.get(), 1, writeen, pFileDest);
	}

	std::cout << "aac encoder time cost : " <<std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - startpts ).count() / 1000.0 <<std::endl;
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
