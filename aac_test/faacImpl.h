#pragma once

#include "../faac/include/faac.h"
#ifdef _DEBUG
#pragma comment(lib,"../faac/x86/Debug/libfaac.lib")
#else
#pragma comment(lib,"../faac/x86/Release/libfaac.lib")
#endif

class faacImpl
{
public:
	faacImpl();
	~faacImpl();

	static void getVersion(char **faac_id_string,
		char **faac_copyright_string);

	void initEncoder(unsigned long sampleRate,
		unsigned int numChannels,
		unsigned long *inputSamples,
		unsigned long *maxOutputBytes);

	void uninitEncoder();

	faacEncConfigurationPtr getEncConfigration();
	int setEncConfigituration(faacEncConfigurationPtr config);

	int aacEncode(int32_t *inputBuffer,
		unsigned int samplesInput,
		unsigned char* outputBuffer,
		unsigned int bufferSize);

private:
	faacEncHandle aacHandle = nullptr;
};

