#ifndef VPDFILE_H
#define VPDFILE_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <vpvl/Common.h>

namespace vpvl {
class Bone;
class PMDModel;
}

class VPDFile
{
public:
    struct Bone
    {
        QString name;
        vpvl::Vector3 position;
        vpvl::Vector4 rotation;
    };
    typedef QList<Bone *> BoneList;

    /**
      * Type of parsing errors.
      */
    enum Error
    {
        kNoError,
        kInvalidHeaderError,
        kInvalidSignatureError,
        kBoneNameError,
        kPositionError,
        kQuaternionError,
        kEndError,
        kMaxErrors
    };

    VPDFile();
    ~VPDFile();

    bool load(QTextStream &stream);
    void save(QTextStream &stream);
    void makePose(vpvl::PMDModel *model);
    VPDFile *clone();

    const BoneList &bones() const { return m_bones; }
    void setBones(const BoneList &value) { m_bones = value; }

private:
    BoneList m_bones;
    Error m_error;
};

#endif // VPDFILE_H
