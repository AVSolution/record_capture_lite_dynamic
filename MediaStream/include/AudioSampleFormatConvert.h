//
//  AudioSampleFormatConvert.h
//  MediaStreamer
//
//  Created by Think on 2019/10/15.
//  Copyright © 2019 Cell. All rights reserved.
//

#ifndef AudioSampleFormatConvert_h
#define AudioSampleFormatConvert_h

#include <math.h>

#define CPU_CLIPS_NEGATIVE 0
#define CPU_CLIPS_POSITIVE 0

inline void src_short_to_float_array (const short *in, float *out, int len)
{
    while (len)
    {
        len-- ;
        out [len] = (float) (in [len] / (1.0 * 0x8000)) ;
    }
}

inline void src_float_to_short_array (const float *in, short *out, int len)
{
    double scaled_value ;

    while (len)
    {
        len-- ;

        scaled_value = in [len] * (8.0 * 0x10000000) ;
        if (CPU_CLIPS_POSITIVE == 0 && scaled_value >= (1.0 * 0x7FFFFFFF))
        {
            out [len] = 32767;
            continue;
        }
        
        if (CPU_CLIPS_NEGATIVE == 0 && scaled_value <= (-8.0 * 0x10000000))
        {
            out [len] = -32768;
            continue;
        }

        out [len] = (short) (lrint (scaled_value) >> 16);
    }
}
#endif /* AudioSampleFormatConvert_h */
