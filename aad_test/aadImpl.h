#pragma once

#include "../faad/include/faad.h"
#ifdef _DEBUG
#pragma comment(lib, "../faad/x86/Debug/libfaad.lib")
#else
#pragma comment(lib, "../faad/x86/Release/libfaad.lib")
#endif

class aadImpl
{
public:
	aadImpl();
	~aadImpl();
	
	static int aadDecGetVersion(char **faad_id_string,char **faad_copyright_string);

	long initAACDec(
		unsigned char *buffer,
		unsigned long buffer_size,
		unsigned long *samplerate,
		unsigned char *channels);
	void uninitAACDec();

	NeAACDecConfigurationPtr getAADConfigurationPtr();
	unsigned char setAACDecConfigurationPtr(NeAACDecConfigurationPtr pConfig);

	void* NEAACDecDecode(
		NeAACDecFrameInfo *hInfo,
		unsigned char *buffer,
		unsigned long buffer_size);
	
private:
	NeAACDecHandle aadHandle;
};

