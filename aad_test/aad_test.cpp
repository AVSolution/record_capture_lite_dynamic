// aad_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "aadImpl.h"
#include <chrono>

int main()
{
	getchar();
	auto beginPts = std::chrono::high_resolution_clock::now();
	char * faad_id, *faad_copyright;
	aadImpl::aadDecGetVersion(&faad_id, &faad_copyright);
	std::cout << "id:" << faad_id<< " faad_copy"<<faad_copyright<<std::endl;
	
	aadImpl aadImplInstance;
	uint8_t* bufferSrc = nullptr;
	unsigned long buffer_size = 0;
	unsigned long samplerate = 44100;
	unsigned char channels = 0;
	aadImplInstance.initAACDec(bufferSrc,buffer_size,&samplerate,&channels);

	NeAACDecConfigurationPtr pConfig = aadImplInstance.getAADConfigurationPtr();
	pConfig->defObjectType = LC;
	pConfig->defSampleRate = samplerate;
	pConfig->outputFormat = FAAD_FMT_16BIT;
	pConfig->dontUpSampleImplicitSBR = 1;
	aadImplInstance.setAACDecConfigurationPtr(pConfig);

	//¶ÁÈ¡adts 0xfff. ÅÐ¶Ï.

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
