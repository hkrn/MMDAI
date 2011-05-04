/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "QMATimer.h"

#include <QDebug>

QMATimer::QMATimer()
    : m_systemStartTime(0),
      m_lastUpdateFrameSystem(0.0f),
      m_pauseTime(0),
      m_framePerSecond(0.0f),
      m_framePerSecondStartTime(0),
      m_framePerSecondCount(0),
      m_targetAdjustmentFrame(0.0f),
      m_currentAdjustmentFrame(0.0f),
      m_enableAdjustment(false),
      m_userStartTime(0)
{
}

QMATimer::~QMATimer()
{
    m_systemStartTime = 0;
    m_lastUpdateFrameSystem = 0.0f;
    m_pauseTime = 0;
    m_framePerSecond = 0.0f;
    m_framePerSecondStartTime = 0;
    m_framePerSecondCount = 0;
    m_targetAdjustmentFrame = 0.0f;
    m_currentAdjustmentFrame = 0.0f;
    m_enableAdjustment = false;
    m_userStartTime = 0;
}

void QMATimer::initialize()
{
    m_timer.start();
}

void QMATimer::start()
{
    m_userStartTime = m_timer.elapsed();
}

void QMATimer::pause()
{
    m_pauseTime = m_timer.elapsed();
}

void QMATimer::resume()
{
    m_lastUpdateFrameSystem += (m_timer.elapsed() - m_pauseTime) * 0.03f;
}

float QMATimer::getFrameInterval()
{
    int currentFrame = m_timer.elapsed();
    float currentFrameSystem = (currentFrame - m_systemStartTime) * 0.03;
    float intervalFrame = currentFrameSystem - m_lastUpdateFrameSystem;
    m_lastUpdateFrameSystem = currentFrameSystem;
    return intervalFrame;
}

float QMATimer::ellapsed()
{
    return m_timer.elapsed() - m_userStartTime;
}

void QMATimer::countFrame()
{
    m_framePerSecondCount++;
    int t = m_timer.elapsed();
    if (t - m_framePerSecondStartTime >= 1000) {
        m_framePerSecond = 1000.0f * m_framePerSecondCount / static_cast<float>(t - m_framePerSecondStartTime);
        m_framePerSecondStartTime = t;
        m_framePerSecondCount = 0;
    }
}

float QMATimer::getFramePerSecond()
{
    return m_framePerSecond;
}

void QMATimer::setAdjustment(float frame)
{
    m_targetAdjustmentFrame = frame;
}

void QMATimer::startAdjustment()
{
    m_currentAdjustmentFrame = 0.0f;
    m_enableAdjustment = true;
}

void QMATimer::stopAdjustment()
{
    m_enableAdjustment = false;
}

float QMATimer::getCurrentAdjustmentFrame()
{
    return m_currentAdjustmentFrame;
}

float QMATimer::getAdjustmentFrame(float baseFrame)
{
    if (!m_enableAdjustment)
        return 0.0f;
    float mstep = 0.0f;
    if (m_targetAdjustmentFrame > m_currentAdjustmentFrame) {
        mstep = baseFrame > 0.3f ? 0.3f : - baseFrame;
        if (m_currentAdjustmentFrame + mstep > m_targetAdjustmentFrame) {
            mstep = m_targetAdjustmentFrame - m_currentAdjustmentFrame;
            m_currentAdjustmentFrame = m_targetAdjustmentFrame;
        }
        else {
            m_currentAdjustmentFrame += mstep;
        }
    }
    if (m_targetAdjustmentFrame < m_currentAdjustmentFrame) {
        mstep = baseFrame > 0.3f ? -0.15f : - (baseFrame * 0.5f);
        if (m_currentAdjustmentFrame + mstep < m_targetAdjustmentFrame) {
            mstep = m_targetAdjustmentFrame - m_currentAdjustmentFrame;
            m_currentAdjustmentFrame = m_targetAdjustmentFrame;
        }
        else {
            m_currentAdjustmentFrame += mstep;
        }
    }
    return mstep;
}
