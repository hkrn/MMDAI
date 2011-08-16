#include "VPDFile.h"
#include <vpvl/vpvl.h>
#include <QtCore/QtCore>

enum InternalParseState
{
    kNone,
    kPosition,
    kQuaternion,
    kEnd
};

VPDFile::VPDFile()
{
}

VPDFile::~VPDFile()
{
    qDeleteAll(m_bones);
    m_error = kNoError;
}

bool VPDFile::load(QTextStream &stream)
{
    QString line;
    stream.setCodec("Shift-JIS");
    if (stream.readLine() != "Vocaloid Pose Data file") {
        m_error = kInvalidSignatureError;
        return false;
    }
    while (line.isEmpty())
        line = stream.readLine();
    line = QString(stream.readLine().split(QRegExp("\\s+"), QString::SkipEmptyParts).at(0)).remove(';');
    //.remove(lastSemiColonRegExp);
    int nbones = line.toInt();
    if (nbones == 0) {
        m_error = kInvalidHeaderError;
        return false;
    }
    line = "";
    while (line.isEmpty())
        line = stream.readLine();

    QStringList tokens;
    while (!line.isNull()) {
        tokens.append(line.trimmed().split(QRegExp("[\r\n\t]"), QString::SkipEmptyParts));
        line = stream.readLine();
    }

    try {
        InternalParseState state = kNone;
        Bone *bone = 0;
        foreach (QString token, tokens) {
            if (token.startsWith("//"))
                continue;
            switch (state) {
            case kNone: {
                if (token.size() > 6 && token.startsWith("Bone")) {
                    token = token.remove(QRegExp("^Bone\\d+\\{*"));
                    bone = new Bone();
                    bone->name = token;
                    bone->position.setZero();
                    bone->rotation.setZero();
                    m_bones.append(bone);
                    state = kPosition;
                }
                else {
                    throw kBoneNameError;
                }
                break;
            }
            case kPosition: {
                QStringList xyz = token.split(",");
                if (xyz.count() == 3 && bone) {
                    float x = xyz.at(0).toFloat();
                    float y = xyz.at(1).toFloat();
                    float z = QString(xyz.at(2)).remove(';').toFloat();
#ifdef VPVL_COORDINATE_OPENGL
                    bone->position.setValue(x, y, -z);
#else
                    bone->position.setValue(x, y, z);
#endif
                    state = kQuaternion;
                }
                else {
                    throw kPositionError;
                }
                break;
            }
            case kQuaternion:
            {
                QStringList xyzw = token.split(",");
                if (xyzw.count() == 4 && bone) {
                    float x = xyzw.at(0).toFloat();
                    float y = xyzw.at(1).toFloat();
                    float z = xyzw.at(2).toFloat();
                    float w = QString(xyzw.at(3)).remove(';').toFloat();
#ifdef VPVL_COORDINATE_OPENGL
                    bone->rotation.setValue(-x, -y, z, w);
#else
                    bone->rotation.setValue(x, y, z, w);
#endif
                    state = kEnd;
                }
                else {
                    throw kQuaternionError;
                }
                break;
            }
            case kEnd:
            {
                if (token[0] == '}')
                    state = kNone;
                else
                    throw kEndError;
            }
            }
        }
    } catch (Error e) {
        m_error = e;
        return false;
    }

    if (nbones != m_bones.count())
        return false;

    return true;
}

void VPDFile::save(QTextStream & /* stream */)
{
}

void VPDFile::makePose(vpvl::PMDModel *model)
{
    QByteArray bytes;
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    foreach (Bone *b, m_bones) {
        bytes = codec->fromUnicode(b->name);
        vpvl::Bone *bone = model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
        if (bone) {
            btVector3 pos = b->position;
            btVector4 rot = b->rotation;
            const btQuaternion rotation(rot.x(), rot.y(), rot.z(), rot.w());
            bone->setPosition(pos);
            bone->setRotation(rotation);
        }
    }
}

