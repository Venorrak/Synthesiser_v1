/*
Copyright (c) 2023 Electrosmith, Corp, Barry Vercoe, John ffitch

Use of this source code is governed by the LGPL V2.1
license that can be found in the LICENSE file or at
https://opensource.org/license/lgpl-2-1/
*/

#pragma once
#ifndef DSY_ALLPASS_H
#define DSY_ALLPASS_H

#include <stdint.h>
#ifdef __cplusplus

#include <math.h>

/** @file allpass.h */

namespace daisysp
{
/**  
       Allpass filter module \n 
       Passes all frequencies at their original levels, with a phase shift. \n 
*/
class Allpass
{
  public:
    Allpass() {}
    ~Allpass() {}

    /**         
        Initializes the allpass module.
        \param sample_rate  The sample rate of the audio engine being run.
    \param buff Buffer for allpass to use.
    \param size Size of buff.
    */
    void Init(float sample_rate, float* buff, size_t size);

    /** 
     \param in Input sample.
     \return Next floating point sample.
    */
    float Process(float in);

    /**
       Sets the filter frequency (Implemented by delay time).
       \param looptime Filter looptime in seconds.
    */
    void SetFreq(float looptime);

    /**
        \param revtime Reverb time in seconds.
    */
    inline void SetRevTime(float revtime) { rev_time_ = revtime; }


  private:
    float  sample_rate_, rev_time_, loop_time_, prvt_, coef_, max_loop_time_;
    float* buf_;
    int    buf_pos_, mod_;
};
} // namespace daisysp
#endif
#endif
