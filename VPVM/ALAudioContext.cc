/**

 Copyright (c) 2010-2013  hkrn

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

#include "ALAudioContext.h"

#include <QtCore>
#include <vpvl2/Common.h>

#if defined(Q_OS_WIN32)
#define ALURE_STATIC_LIBRARY
#endif
#include <AL/alure.h>

/* alway uses OpenAL soft */
#define AL_ALEXT_PROTOTYPES
#include <AL/alext.h>
#undef AL_ALEXT_PROTOTYPES

ALAudioContext::ALAudioContext(QObject *parent)
    : QObject(parent),
      m_deviceAvailable(false)
{
    connect(this, &ALAudioContext::currentDeviceChanged, this, &ALAudioContext::deviceAvailableChanged);
}

ALAudioContext::~ALAudioContext()
{
    if (m_deviceAvailable) {
        alureShutdownDevice();
    }
}

void ALAudioContext::initialize()
{
    if (!m_initialized) {
        int ndevices = 0;
        const char **devices = alureGetDeviceNames(AL_TRUE, &ndevices);
        for (int i = 0; i < ndevices; i++) {
            const QString &device = QString::fromUtf8(devices[i]);
            VPVL2_VLOG(1, "device[" << i << "] = " << device.toStdString());
            m_devices.append(device);
        }
        alureFreeDeviceNames(devices);
        m_deviceAvailable = alureInitDevice(0, 0) == AL_TRUE;
        m_initialized = true;
        emit deviceAvailableChanged();
    }
}

QStringList ALAudioContext::availableDevices() const
{
    return m_devices;
}

QString ALAudioContext::currentDevice() const
{
    return m_currentDevice;
}

void ALAudioContext::setCurrentDevice(const QString &value)
{
    if (value != m_currentDevice) {
        if (m_deviceAvailable) {
            alureShutdownDevice();
        }
        m_deviceAvailable = alureInitDevice(value.toUtf8().constData(), 0);
        emit currentDeviceChanged();
    }
}

bool ALAudioContext::deviceAvailable() const
{
    return m_deviceAvailable;
}

