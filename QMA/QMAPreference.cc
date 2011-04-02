/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include <MMDAI/MMDAI.h>
#include "QMAPreference.h"

#include <QVector3D>
#include <QVector4D>

QMAPreference::QMAPreference(QSettings *settings)
    : m_settings(settings)
{
}

QMAPreference::~QMAPreference()
{
}

void QMAPreference::load()
{
}

bool QMAPreference::getBool(const MMDAI::PreferenceKeys key)
{
    if (validateBoolKey(key)) {
        return m_values.value(key, getDefaultValue(key)).toBool();
    }
    else {
        MMDAILogWarn("%d is not bool key", key);
        return false;
    }
}

float QMAPreference::getFloat(const MMDAI::PreferenceKeys key)
{
    if (validateFloatKey(key)) {
        return m_values.value(key, getDefaultValue(key)).toFloat();
    }
    else {
        MMDAILogWarn("%d is not float key", key);
        return 0.0f;
    }
}

void QMAPreference::getFloat3(const MMDAI::PreferenceKeys key, float *values)
{
    if (validateFloat3Key(key)) {
        const QVector3D v = m_values.value(key, getDefaultValue(key).value<QVector3D>()).value<QVector3D>();
        values[0] = v.x();
        values[1] = v.y();
        values[2] = v.z();
    }
    else {
        MMDAILogWarn("%d is not float3 key", key);
    }
}

void QMAPreference::getFloat4(const MMDAI::PreferenceKeys key, float *values)
{
    if (validateFloat4Key(key)) {
        const QVector4D v = m_values.value(key, getDefaultValue(key).value<QVector4D>()).value<QVector4D>();
        values[0] = v.x();
        values[1] = v.y();
        values[2] = v.z();
        values[3] = v.w();
    }
    else {
        MMDAILogWarn("%d is not float4 key", key);
    }
}

int QMAPreference::getInt(const MMDAI::PreferenceKeys key)
{
    if (validateIntKey(key)) {
        return m_values.value(key, getDefaultValue(key)).toInt();
    }
    else {
        MMDAILogWarn("%d is not int key", key);
        return 0;
    }
}

void QMAPreference::setBool(const MMDAI::PreferenceKeys key, bool value)
{
    if (validateBoolKey(key)) {
        m_values.insert(key, QVariant(value));
    }
    else {
        MMDAILogWarn("%d is not bool key", key);
    }
}

void QMAPreference::setFloat(const MMDAI::PreferenceKeys key, float value)
{
    Q_UNUSED(value);
    if (validateFloatKey(key)) {
        QVariant var(value);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not float key", key);
    }
}

void QMAPreference::setFloat3(const MMDAI::PreferenceKeys key, float *values)
{
    Q_UNUSED(values);
    if (validateFloat3Key(key)) {
        QVector3D vec3;
        vec3.setX(values[0]);
        vec3.setY(values[1]);
        vec3.setZ(values[2]);
        QVariant var(vec3);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not float3 key", key);
    }
}

void QMAPreference::setFloat4(const MMDAI::PreferenceKeys key, float *values)
{
    Q_UNUSED(values);
    if (validateFloat4Key(key)) {
        QVector4D vec4;
        vec4.setX(values[0]);
        vec4.setY(values[1]);
        vec4.setZ(values[2]);
        vec4.setW(values[3]);
        QVariant var(vec4);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not float4 key", key);
    }
}

void QMAPreference::setInt(const MMDAI::PreferenceKeys key, int value)
{
    Q_UNUSED(value);
    if (validateIntKey(key)) {
        QVariant var(value);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not int key", key);
    }
}

bool QMAPreference::validateBoolKey(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceFullScreen:
    case MMDAI::kPreferenceShadowMappingLightFirst:
    case MMDAI::kPreferenceShowFPS:
    case MMDAI::kPreferenceTopMost:
    case MMDAI::kPreferenceUseCartoonRendering:
    case MMDAI::kPreferenceUseMMDLikeCartoon:
    case MMDAI::kPreferenceUseShadowMapping:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateFloatKey(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceCartoonEdgeStep:
    case MMDAI::kPreferenceLogScale:
    case MMDAI::kPreferenceLightIntensity:
    case MMDAI::kPreferenceRotateStep:
    case MMDAI::kPreferenceScaleStep:
    case MMDAI::kPreferenceShadowMappingFloorDensity:
    case MMDAI::kPreferenceShadowMappingSelfDensity:
    case MMDAI::kPreferenceTranslateStep:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateFloat3Key(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceCampusColor:
    case MMDAI::kPreferenceFPSPosition:
    case MMDAI::kPreferenceLightColor:
    case MMDAI::kPreferenceStageSize:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateFloat4Key(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceLightDirection:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateIntKey(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceBulletFPS:
    case MMDAI::kPreferenceShadowMappingTextureSize:
    case MMDAI::kPreferenceMaxMultiSampling:
        return true;
    default:
        return false;
    }
}

QVariant QMAPreference::getDefaultValue(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceBulletFPS:
        return QVariant(120);
    case MMDAI::kPreferenceCampusColor:
        return QVariant(QVector3D(0.0f, 0.0f, 0.0f));
    case MMDAI::kPreferenceCartoonEdgeStep:
        return QVariant(1.2f);
    case MMDAI::kPreferenceCartoonEdgeWidth:
        return QVariant(0.7f);
    case MMDAI::kPreferenceFPSPosition:
        return QVariant(QVector3D(-2.5f, 22.0f, 3.0f));
    case MMDAI::kPreferenceFullScreen:
        return QVariant(false);
    case MMDAI::kPreferenceLightColor:
        return QVariant(QVector3D(1.0f, 1.0f, 1.0f));
    case MMDAI::kPreferenceLightDirection:
        return QVariant(QVector4D(0.5f, 1.0f, 0.5f, 0.0f));
    case MMDAI::kPreferenceLightIntensity:
        return QVariant(0.6f);
    case MMDAI::kPreferenceLogPosition:
        return QVariant(QVector3D(-17.5f, 3.0f, -15.0f));
    case MMDAI::kPreferenceLogScale:
        return QVariant(1.0f);
    case MMDAI::kPreferenceLogSize:
        return QVariant(-1);
    case MMDAI::kPreferenceMaxMultiSampling:
        return QVariant(4);
    case MMDAI::kPreferenceMaxMultiSamplingColor:
        return QVariant(4);
    case MMDAI::kPreferenceMotionAdjustFrame:
        return QVariant(0);
    case MMDAI::kPreferenceRotateStep:
        return QVariant(0.08f);
    case MMDAI::kPreferenceScaleStep:
        return QVariant(1.05f);
    case MMDAI::kPreferenceShadowMappingFloorDensity:
        return QVariant(0.5f);
    case MMDAI::kPreferenceShadowMappingLightFirst:
        return QVariant(true);
    case MMDAI::kPreferenceShadowMappingSelfDensity:
        return QVariant(1.0f);
    case MMDAI::kPreferenceShadowMappingTextureSize:
        return QVariant(1024);
    case MMDAI::kPreferenceShowFPS:
        return QVariant(true);
    case MMDAI::kPreferenceStageSize:
        return QVariant(-1);
    case MMDAI::kPreferenceTopMost:
        return QVariant(false);
    case MMDAI::kPreferenceTranslateStep:
        return QVariant(0.5f);
    case MMDAI::kPreferenceUseCartoonRendering:
        return QVariant(true);
    case MMDAI::kPreferenceUseMMDLikeCartoon:
        return QVariant(true);
    case MMDAI::kPreferenceUseShadowMapping:
        return QVariant(false);
    case MMDAI::kPreferenceWindowSize:
        return QVariant(-1);
    default:
        MMDAILogWarn("should not reach here: %d", key);
        return QVariant(-1);
    }
}

void QMAPreference::round(const MMDAI::PreferenceKeys key, QVariant &value)
{
    QVector3D vec3;
    switch (key) {
    case MMDAI::kPreferenceBulletFPS:
        value.setValue(qMin(qMax(value.toInt(), 1), 120));
        break;
    case MMDAI::kPreferenceCampusColor:
        vec3 = value.value<QVector3D>();
        vec3.setX(qMin(qMax(vec3.x(), 0.0), 1.0));
        vec3.setY(qMin(qMax(vec3.y(), 0.0), 1.0));
        vec3.setZ(qMin(qMax(vec3.z(), 0.0), 1.0));
        value.setValue(vec3);
        break;
    case MMDAI::kPreferenceCartoonEdgeStep:
        value.setValue(qMin(qMax(value.toFloat(), 1.0f), 10.0f));
        break;
    case MMDAI::kPreferenceCartoonEdgeWidth:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceLightColor:
        vec3 = value.value<QVector3D>();
        vec3.setX(qMin(qMax(vec3.x(), 0.0), 1.0));
        vec3.setY(qMin(qMax(vec3.y(), 0.0), 1.0));
        vec3.setZ(qMin(qMax(vec3.z(), 0.0), 1.0));
        value.setValue(vec3);
        break;
    case MMDAI::kPreferenceLightIntensity:
        value.setValue(qMin(qMax(value.toFloat(), 0.0f), 1.0f));
        break;
    case MMDAI::kPreferenceLogScale:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceLogSize:
        // TODO
        break;
    case MMDAI::kPreferenceMaxMultiSampling:
        value.setValue(qMin(qMax(value.toInt(), 0), 32));
        break;
    case MMDAI::kPreferenceMaxMultiSamplingColor:
        value.setValue(qMin(qMax(value.toInt(), 0), 32));
        break;
    case MMDAI::kPreferenceRotateStep:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceScaleStep:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceShadowMappingFloorDensity:
        value.setValue(qMin(qMax(value.toFloat(), 0.0f), 1.0f));
        break;
    case MMDAI::kPreferenceShadowMappingSelfDensity:
        value.setValue(qMin(qMax(value.toFloat(), 0.0f), 1.0f));
        break;
    case MMDAI::kPreferenceShadowMappingTextureSize:
        value.setValue(qMin(qMax(value.toInt(), 1), 8192));
        break;
    case MMDAI::kPreferenceStageSize:
        // TODO
        break;
    case MMDAI::kPreferenceTranslateStep:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceWindowSize:
        // TODO
        break;
    default:
        break;
    }
}
