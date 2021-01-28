#include "aadImpl.h"

aadImpl::aadImpl():aadHandle(nullptr)
{

}

aadImpl::~aadImpl()
{
	uninitAACDec();
}

int aadImpl::aadDecGetVersion(char **faad_id_string,
	char **faad_copyright_string)
{
	return NeAACDecGetVersion(faad_id_string, faad_copyright_string);
}

long aadImpl::initAACDec(
	unsigned char *buffer,
	unsigned long buffer_size,
	unsigned long *samplerate,
	unsigned char *channels)
{
	if (aadHandle == nullptr) {
		aadHandle = NeAACDecOpen();

		return NeAACDecInit(aadHandle, buffer, buffer_size, samplerate, channels);
	}
}

void aadImpl::uninitAACDec()
{
	if (aadHandle)
		NeAACDecClose(aadHandle);
	aadHandle = nullptr;
}

NeAACDecConfigurationPtr aadImpl::getAADConfigurationPtr()
{
	if (aadHandle) {
		return NeAACDecGetCurrentConfiguration(aadHandle);
	}
}

unsigned char aadImpl::setAACDecConfigurationPtr(NeAACDecConfigurationPtr pConfig)
{
	if (aadHandle)
		return NeAACDecSetConfiguration(aadHandle, pConfig);
}

void* aadImpl::NEAACDecDecode(
	NeAACDecFrameInfo *hInfo,
	unsigned char *buffer,
	unsigned long buffer_size)
{
	if (aadHandle) {
		return NeAACDecDecode(aadHandle, hInfo, buffer, buffer_size);
	}
}