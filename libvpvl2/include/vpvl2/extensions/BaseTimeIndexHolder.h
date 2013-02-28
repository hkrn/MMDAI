/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#pragma once
#ifndef VPVL2_EXTENSIONS_BASETIMEINDEXHOLDER_H_
#define VPVL2_EXTENSIONS_BASETIMEINDEXHOLDER_H_

#include <vpvl2/Common.h>
#include <vpvl2/IKeyframe.h>

namespace vpvl2
{
namespace extensions
{

class BaseTimeIndexHolder {
public:
    BaseTimeIndexHolder()
        : m_previousTimeIndex(0),
          m_updateInterval(0)
    {
    }
    virtual ~BaseTimeIndexHolder() {
        m_currentElapsed = 0;
        m_previousTimeIndex = 0;
        m_updateInterval = 0;
    }

    void start() {
        timerStart();
    }
    void reset() {
        timerReset();
        m_currentElapsed = 0;
        m_previousTimeIndex = 0;
    }
    IKeyframe::TimeIndex delta() const {
        const IKeyframe::TimeIndex elapsed(timeIndex());
        IKeyframe::TimeIndex delta(elapsed - m_previousTimeIndex);
        m_previousTimeIndex = elapsed;
        if (delta < 0) {
            delta = elapsed;
        }
        return delta;
    }
    IKeyframe::TimeIndex elapsed() const {
        return IKeyframe::TimeIndex(m_currentElapsed);
    }
    IKeyframe::TimeIndex timeIndex() const {
        return m_currentElapsed / m_updateInterval;
    }
    void setUpdateInterval(const IKeyframe::TimeIndex &value) {
        m_updateInterval = value;
    }
    void saveElapsed() {
        saveElapsed(0);
    }
    void saveElapsed(int64_t value) {
        m_currentElapsed = btMax(timerElapsed(), value);
    }

protected:
    virtual void timerStart() = 0;
    virtual void timerReset() = 0;
    virtual int64_t timerElapsed() const = 0;

private:
    mutable IKeyframe::TimeIndex m_previousTimeIndex;
    IKeyframe::TimeIndex m_updateInterval;
    int64_t m_currentElapsed;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
