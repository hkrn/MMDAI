/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

QMATimer::QMATimer()
  : m_value(0.0),
    m_lastFrame(0.0),
    m_paused(0),
    m_count(0)
{
}

QMATimer::~QMATimer()
{
}

void QMATimer::start()
{
  m_timeFPS.start();
  m_timeFrame.start();
}

void QMATimer::pause()
{
  m_paused = m_timeFPS.elapsed();
}

void QMATimer::resume()
{
  m_lastFrame += (m_timeFPS.elapsed() - m_paused) * 0.03;
}

double QMATimer::getAuxFrame(double base)
{
  base = 0.0;
  return 0.0;
}

void QMATimer::count()
{
  m_count++;
  int t = m_timeFPS.elapsed();
  if (t >= 1000) {
    m_value = 1000.0f * (double) m_count / t;
    m_count = 0;
    m_timeFPS.restart();
  }
}

double QMATimer::getFPS()
{
  return m_value;
}

double QMATimer::getInterval()
{
  int elapsed = m_timeFrame.elapsed();
  double currentFrame = elapsed * 0.03;
  double intervalFrame = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;
  return intervalFrame;
}
