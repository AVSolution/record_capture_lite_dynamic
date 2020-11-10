#include "audioUtil.h"

namespace RL {
	namespace Util {


		int16_t  MixerAddS16(int16_t var1, int16_t var2)
		{
			static const int32_t kMaxInt16 = 32767;
			static const int32_t kMinInt16 = -32768;
			int32_t tmp = (int32_t)var1 + (int32_t)var2;
			int16_t out16;

			if (tmp > kMaxInt16) {
				out16 = kMaxInt16;
			}
			else if (tmp < kMinInt16) {
				out16 = kMinInt16;
			}
			else {
				out16 = (int16_t)tmp;
			}

			return out16;
		}

		void MixerAddS16(int16_t* src1, const int16_t* src2, size_t size)
		{
			for (size_t i = 0; i < size; ++i) {
				src1[i] = MixerAddS16(src1[i], src2[i]);
			}
		}

		void convert32fToS16(const int32_t* src, size_t size, int16_t* dst)
		{
			for (int i = 0; i < size ; i++) {//32 float
				float ff = *(float*)(src + i);
				short ss = ff * 32768;
				memcpy(dst + i , &ss, sizeof(int16_t));
			}
		}


	}//namespace Util
}//namespace RL