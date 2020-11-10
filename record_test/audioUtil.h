#ifndef __AudioUtil_H__
#define __AudioUtil_H__

#include <iostream>

namespace RL {
	namespace Util {

		void MixerAddS16(int16_t* src1, const int16_t* src2, size_t size);
		void convert32fToS16(const int32_t* src, size_t size, int16_t* dst);

	}//namespace Util
}//namespace RL

#endif