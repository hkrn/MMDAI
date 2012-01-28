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

#include "LipSync.h"

#include "util.h"

#include <QtCore/QtCore>
#include <vpvl/vpvl.h>

struct LipKeyFrame
{
    int phone;
    int duration;
    float rate;
};

const float LipSync::kInterpolationRate = 0.8f;

LipSync::LipSync()
{
}

LipSync::~LipSync()
{
    release();
}

bool LipSync::load(QTextStream &stream)
{
    enum State
    {
        GetNExpressions,
        GetExpressionNames,
        GetPhoneNames
    };
    State state = GetNExpressions;
    int i = 0, nexpressions = 0, nphonemes = 0;
    bool ret = false;
    release();
    stream.setCodec("Shift-JIS");
    do {
        QString line = stream.readLine().trimmed();
        if (!line.isEmpty() && line.at(0) != '#') {
            switch (state) {
            case GetNExpressions:
                nexpressions = line.toInt();
                state = GetExpressionNames;
                break;
            case GetExpressionNames:
                if (i < nexpressions) {
                    m_expressionNames.append(line);
                    i++;
                }
                else {
                    nphonemes = line.toInt();
                    state = GetPhoneNames;
                    i = 0;
                }
                break;
            case GetPhoneNames:
                if (i < nphonemes) {
                    QStringList a = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    if (a.count() == 1 + nexpressions) {
                        m_phoneNames.append(a.at(0));
                        for (int j = 0; j < nexpressions; j++) {
                            float f = a.at(j + 1).toFloat();
                            m_interpolation.append(f);
                        }
                        i++;
                    }
                    if (i == nphonemes) {
                        i = 0;
                        ret = true;
                    }
                }
                break;
            }
        }
    } while (!stream.atEnd());
    return ret;
}

vpvl::VMDMotion *LipSync::createMotion(const QString &sequence)
{
    QStringList tokens = sequence.split(',');
    QList<LipKeyFrame> frames, newFrames;
    vpvl::VMDMotion *motion = 0;
    LipKeyFrame frame;
    int i = 0, j = 0, k = 0;
    float diff = 0.0f;
    foreach (QString token, tokens) {
        if (i % 2 == 0) {
            int nPhones = m_phoneNames.size();
            for (j = 0; j < nPhones; j++) {
                if (m_phoneNames[j] == token) {
                    k = j;
                    break;
                }
            }
            if (nPhones <= j)
                k = 0;
        }
        else {
            float msecf = token.toFloat() * 0.03f + diff;
            int msec = static_cast<int>(msecf + 0.5f);
            frame.phone = k;
            frame.duration = qMax(msec, 1);
            frame.rate = 1.0f;
            frames.append(frame);
            diff = msecf - frame.duration;
        }
        i++;
    }
    frame.phone = 0;
    frame.duration = 1;
    frame.rate = 0.0f;
    frames.append(frame);
    foreach (LipKeyFrame f, frames) {
        if (f.duration > kInterpolationMargin) {
            frame.phone = f.phone;
            frame.duration = kInterpolationMargin;
            frame.rate =  f.rate *kInterpolationRate;
            f.duration -= kInterpolationMargin;
            newFrames.append(frame);
        }
        newFrames.append(f);
    }
    motion = new vpvl::VMDMotion();
    vpvl::FaceAnimation *fa = motion->mutableFaceAnimation();
    int nExpressionNames = m_expressionNames.size();
    int currentFrame = 0;
    for (i = 0; i < nExpressionNames; i++) {
        currentFrame = 0;
        QByteArray bytes = internal::fromQString(m_expressionNames.at(i));
        foreach (LipKeyFrame f, newFrames) {
            vpvl::FaceKeyframe *ff = new vpvl::FaceKeyframe();
            ff->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
            ff->setFrameIndex(currentFrame);
            ff->setWeight(blendRate(f.phone, i) * f.rate);
            fa->addKeyframe(ff);
            currentFrame += f.duration;
        }
    }
    return motion;
}

float LipSync::blendRate(int i, int j)
{
    return m_interpolation.at(i * m_expressionNames.size() + j);
}

void LipSync::release()
{
    m_expressionNames.clear();
    m_phoneNames.clear();
    m_interpolation.clear();
}
