/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

#include <QtCore>
#include <QCoreApplication>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

const QColor Util::kRed = QColor(0xff, 0x41, 0x36);
const QColor Util::kGreen = QColor(0x2e, 0xcc, 0x40);
const QColor Util::kBlue = QColor(0x00, 0x74, 0xd9);
const QColor Util::kYellow = QColor(0xff, 0xdc, 0x00);
const QColor Util::kGray = QColor(0xaa, 0xaa, 0xaa);

QString Util::toQString(const IString *value)
{
    return value ? static_cast<const String *>(value)->value() : QString();
}

bool Util::equalsString(const QString lhs, const IString *rhs)
{
    return lhs == static_cast<const String *>(rhs)->value();
}

QMatrix4x4 Util::fromMatrix4(const glm::mat4 &value)
{
    QMatrix4x4 m;
    for (int i = 0; i < 16; i++) {
        m.data()[i] = glm::value_ptr(value)[i];
    }
    return m;
}

Vector3 Util::toVector3(const QVector3D &value)
{
    return Vector3(value.x(), value.y(), value.z());
}

QVector3D Util::fromVector3(const Vector3 &value)
{
    return QVector3D(value.x(), value.y(), value.z());
}

Vector3 Util::toVector3Radian(const QVector3D &value)
{
    return Vector3(qDegreesToRadians(value.x()), qDegreesToRadians(value.y()), qDegreesToRadians(value.z()));
}

QVector3D Util::fromVector3Radian(const Vector3 &value)
{
    return QVector3D(qRadiansToDegrees(value.x()), qRadiansToDegrees(value.y()), qRadiansToDegrees(value.z()));
}

vpvl2::Vector4 Util::toVector4(const QVector4D &value)
{
    return Vector4(value.x(), value.y(), value.z(), value.w());
}

QVector4D Util::fromVector4(const vpvl2::Vector4 &value)
{
    return QVector4D(value.x(), value.y(), value.z(), value.w());
}

Vector3 Util::toColorRGB(const QColor &value)
{
    return Vector3(value.redF(), value.greenF(), value.blueF());
}

QColor Util::fromColorRGB(const Vector3 &value)
{
    return QColor::fromRgbF(value.x(), value.y(), value.z());
}

Color Util::toColorRGBA(const QColor &value)
{
    return Color(value.redF(), value.greenF(), value.blueF(), value.alphaF());
}

QColor Util::fromColorRGBA(const Color &value)
{
    return QColor::fromRgbF(value.x(), value.y(), value.z(), value.w());
}

Quaternion Util::toQuaternion(const QQuaternion &value)
{
    return Quaternion(value.x(), value.y(), value.z(), value.scalar());
}

QQuaternion Util::fromQuaternion(const Quaternion &value)
{
    return QQuaternion(value.w(), value.x(), value.y(), value.z());
}

QString Util::resourcePath(const QString &basePath)
{
    const QString &appPath = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MAC)
    if (!QDir::isAbsolutePath(basePath)) {
        return QDir::cleanPath(QStringLiteral("%1/../Resources/%2").arg(appPath, basePath));
    }
#endif
    return QDir::cleanPath(QDir(appPath).absoluteFilePath(basePath));
}

QJsonValue Util::toJson(const QColor &value)
{
    QJsonArray v;
    v.append(value.redF());
    v.append(value.greenF());
    v.append(value.blueF());
    v.append(value.alphaF());
    return v;
}

QJsonValue Util::toJson(const QVector3D &value)
{
    QJsonArray v;
    v.append(value.x());
    v.append(value.y());
    v.append(value.z());
    return v;
}

QJsonValue Util::toJson(const QVector4D &value)
{
    QJsonArray v;
    v.append(value.x());
    v.append(value.y());
    v.append(value.z());
    v.append(value.w());
    return v;
}

QJsonValue Util::toJson(const QQuaternion &value)
{
    QJsonArray v;
    v.append(value.x());
    v.append(value.y());
    v.append(value.z());
    v.append(value.scalar());
    return v;
}
