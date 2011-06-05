#ifndef VPVL_BASEMOTION_H_
#define VPVL_BASEMOTION_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"

namespace vpvl
{

class BaseMotion
{
public:
    BaseMotion(float smearDefault) :
        m_lastIndex(0),
        m_smearDefault(smearDefault),
        m_maxFrame(0.0f),
        m_currentFrame(0.0f),
        m_previousFrame(0.0f),
        m_lastLoopStartFrame(0.0f),
        m_blendRate(1.0f),
        m_smearIndex(smearDefault),
        m_ignoreSingleMotion(false),
        m_overrideFirst(false)
    {
    }
    virtual ~BaseMotion() {}

    virtual void read(const char *data, uint32_t size) = 0;
    virtual void seek(float frameAt) = 0;
    virtual void takeSnap(const btVector3 &center) = 0;

    void advance(float deltaFrame, bool &reached) {
        seek(m_currentFrame);
        m_previousFrame = m_currentFrame;
        m_currentFrame += deltaFrame;
        if (m_currentFrame >= m_maxFrame) {
            m_currentFrame = m_maxFrame;
            reached = true;
        }
        else {
            reached = false;
        }
    }
    void rewind(float target, float frameAt) {
        static const btVector3 zero(0.0f, 0.0f, 0.0f);
        m_currentFrame = m_previousFrame + frameAt - m_maxFrame + target;
        m_previousFrame = target;
        if (m_overrideFirst) {
            takeSnap(zero);
            m_lastLoopStartFrame = target;
            if (m_maxFrame >= m_smearDefault) {
                m_smearIndex = m_smearDefault;
            }
            else {
                m_smearIndex -= m_maxFrame + 1.0f;
                btSetMax(m_smearIndex, 0.0f);
            }
        }
    }
    void reset() {
        m_currentFrame = 0.0f;
        m_previousFrame = 0.0f;
        m_lastLoopStartFrame = 0.0f;
        m_blendRate = 0.0f;
        m_smearIndex = m_smearDefault;
    }
    void setOverrideFirst(const btVector3 &center) {
        takeSnap(center);
        m_overrideFirst = true;
        m_smearIndex = m_smearDefault;
    }

    float blendRate() const {
        return m_blendRate;
    }
    float maxIndex() const {
        return m_maxFrame;
    }
    void setBlendRate(float value) {
        m_blendRate = value;
    }

protected:
    uint32_t m_lastIndex;
    const float m_smearDefault;
    float m_maxFrame;
    float m_currentFrame;
    float m_previousFrame;
    float m_lastLoopStartFrame;
    float m_blendRate;
    float m_smearIndex;
    bool m_ignoreSingleMotion;
    bool m_overrideFirst;
};

}

#endif

