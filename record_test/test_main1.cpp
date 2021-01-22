#include "recordimpl.h"

#define RECORD_AUDIO_RESAMPLE_SAMPLERATE  44100
#define RECORD_AUDIO_RESAMPLE_CHANNEL 2
#define RECORD_AUDIO_RESAMPLE_BYTEPERSAMPLE 2

int main()
{
	int nums = getchar() - '0';
	CRecordImpl* pRecordImpl = CRecordImpl::getInstance();

	std::string  recordPath = "C:\\tmp\\cris_temp.mp4";
	VideoParam videoparam{1600,950,10,1024};
	AudioParam audioParam{ 
		RECORD_AUDIO_RESAMPLE_SAMPLERATE ,
		RECORD_AUDIO_RESAMPLE_CHANNEL,
		RECORD_AUDIO_RESAMPLE_BYTEPERSAMPLE ,128};
	pRecordImpl->startRecord(recordPath,videoparam,audioParam);
	
	int i = 0;
	while (++i < 5 * nums) {
		pRecordImpl->logInstance->rlog(RLOG::LOG_DEBUG, "Sleep 2 seconds");
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	pRecordImpl->stopRecord();

	pRecordImpl->logInstance->rlog(RL::RecordCapture::IRecordLog::LOG_INFO, "%s", "=====record end........\n");


	return 0;
}