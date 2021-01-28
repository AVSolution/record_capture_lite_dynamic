#include "faacImpl.h"

faacImpl::faacImpl():aacHandle(nullptr)
{

}

faacImpl::~faacImpl()
{
	uninitEncoder();
}

void faacImpl::getVersion(char **faac_id_string,
	char **faac_copyright_string)
{
	faacEncGetVersion(faac_id_string, faac_copyright_string);
}

void faacImpl::initEncoder(unsigned long sampleRate,
	unsigned int numChannels,
	unsigned long *inputSamples,
	unsigned long *maxOutputBytes)
{
	if (aacHandle == nullptr) {
		aacHandle  = faacEncOpen(sampleRate, numChannels, inputSamples, maxOutputBytes);
		if (aacHandle) {
			faacEncConfigurationPtr pFec = faacEncConfigurationPtr(aacHandle);
			if (pFec) {
				
			}
		}
	}
}

void faacImpl::uninitEncoder()
{
	if (aacHandle) {
		faacEncClose(aacHandle);
		aacHandle = nullptr;
	}
}

faacEncConfigurationPtr faacImpl::getEncConfigration()
{
	if (aacHandle)
		return faacEncGetCurrentConfiguration(aacHandle);
}

int faacImpl::setEncConfigituration(faacEncConfigurationPtr config)
{
	if (aacHandle)
		return faacEncSetConfiguration(aacHandle, config);
}

int faacImpl::aacEncode(int32_t *inputBuffer,
	unsigned int samplesInput,
	unsigned char* outputBuffer,
	unsigned int bufferSize)
{
	if (aacHandle) {
		return faacEncEncode(aacHandle, inputBuffer, samplesInput, outputBuffer, bufferSize);
	}
}