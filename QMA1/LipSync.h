#ifndef LIPSYNC_H
#define LIPSYNC_H

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

namespace vpvl
{
class VMDMotion;
}

class LipSync
{
public:
    static const int kInterpolationMargin = 2;
    static const float kInterpolationRate;

    LipSync();
    ~LipSync();

    bool load(QTextStream &stream);
    vpvl::VMDMotion *createMotion(const QString &sequence);

private:
    float blendRate(int i, int j);
    void release();

    QStringList m_expressionNames;
    QStringList m_phoneNames;
    QList<float> m_interpolation;
};

#endif // LIPSYNC_H
