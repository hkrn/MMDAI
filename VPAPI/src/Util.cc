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

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

#include "Util.h"

#include <QCoreApplication>
#include <QDir>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

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

Vector3 Util::toColor(const QColor &value)
{
    return Vector3(value.redF(), value.greenF(), value.blueF());
}

QColor Util::fromColor(const Vector3 &value)
{
    return QColor::fromRgbF(value.x(), value.y(), value.z());
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
