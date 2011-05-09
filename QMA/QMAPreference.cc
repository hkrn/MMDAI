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

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>

QMAPreference::QMAPreference(QSettings *settings)
    : m_basename("MMDAI"),
      m_settings(settings)
{
}

QMAPreference::~QMAPreference()
{
}

void QMAPreference::load(QFile &file)
{
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        for (QString line = stream.readLine(); !line.isNull(); line = stream.readLine()) {
            line = line.trimmed();
            if (line.isEmpty() || line[0] == '#') {
                continue;
            }
            QStringList pair = line.split('=');
            if (pair.size() != 2) {
                continue;
            }
            QString key = pair[0];
            QString value = pair[1];
            parse(key, value);
        }
        file.close();
        m_basename = QFileInfo(file).baseName();
    }
    else {
        MMDAILogWarn("Cannot open %s: %s", file.fileName().toUtf8().constData(),
                     file.errorString().toUtf8().constData());
    }
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
    if (validateFloatKey(key)) {
        QVariant var(value);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not float key", key);
    }
}

void QMAPreference::setFloat3(const MMDAI::PreferenceKeys key, const float *values)
{
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

void QMAPreference::setFloat4(const MMDAI::PreferenceKeys key, const float *values)
{
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
    if (validateIntKey(key)) {
        QVariant var(value);
        round(key, var);
        m_values.insert(key, var);
    }
    else {
        MMDAILogWarn("%d is not int key", key);
    }
}

static void QMAPreferenceValue2FloatValues(const QString &value, float *values, int size) {
    QStringList v = value.split(',');
    if (v.length() == size) {
        for (int i = 0; i < size; i++) {
            values[i] = v[i].toFloat();
        }
    }
    else {
        memset(values, 0, sizeof(float) * size);
    }
}

static void QMAPreferenceValue2Float3(const QString &value, float *values) {
    QMAPreferenceValue2FloatValues(value, values, 3);
}

static void QMAPreferenceValue2Float4(const QString &value, float *values) {
    QMAPreferenceValue2FloatValues(value, values, 4);
}

void QMAPreference::parse(const QString &key, const QString &value)
{
    float vec3[3], vec4[4];
    if (key == "use_cartoon_rendering") {
        setBool(MMDAI::kPreferenceUseCartoonRendering, value.toLower() == "true");
    }
    else if (key == "use_mmd_like_cartoon") {
        setBool(MMDAI::kPreferenceUseMMDLikeCartoon, value.toLower() == "true");
    }
    else if (key == "cartoon_edge_width") {
        setFloat(MMDAI::kPreferenceCartoonEdgeWidth, value.toFloat());
    }
    else if (key == "cartoon_edge_step") {
        setFloat(MMDAI::kPreferenceCartoonEdgeStep, value.toFloat());
    }
    else if (key == "cartoon_edge_selected_color") {
        QMAPreferenceValue2Float4(value, vec4);
        setFloat4(MMDAI::kPreferenceCartoonEdgeSelectedColor, vec4);
    }
    else if (key == "camera_rotation") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceCameraRotation, vec3);
    }
    else if (key == "camera_transition") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceCameraTransition, vec3);
    }
    else if (key == "camera_distance") {
        setFloat(MMDAI::kPreferenceCameraDistance, value.toFloat());
    }
    else if (key == "camera_fovy") {
        setFloat(MMDAI::kPreferenceCameraFovy, value.toFloat());
    }
    else if (key == "show_fps") {
        setBool(MMDAI::kPreferenceShowFPS, value.toLower() == "true");
    }
    else if (key == "fps_position") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceFPSPosition, vec3);
    }
    else if (key == "top_most") {
        setBool(MMDAI::kPreferenceTopMost, value.toLower() == "true");
    }
    else if (key == "full_screen") {
        setBool(MMDAI::kPreferenceFullScreen, value.toLower() == "true");
    }
    else if (key == "log_position") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceLogPosition, vec3);
    }
    else if (key == "log_scale") {
        setFloat(MMDAI::kPreferenceLogScale, value.toFloat());
    }
    else if (key == "light_direction") {
        QMAPreferenceValue2Float4(value, vec4);
        setFloat4(MMDAI::kPreferenceLightDirection, vec4);
    }
    else if (key == "light_intensity") {
        setFloat(MMDAI::kPreferenceLightIntensity, value.toFloat());
    }
    else if (key == "light_color") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceLightColor, vec3);
    }
    else if (key == "campus_color") {
        QMAPreferenceValue2Float3(value, vec3);
        setFloat3(MMDAI::kPreferenceCampusColor, vec3);
    }
    else if (key == "max_multi_sampling") {
        setInt(MMDAI::kPreferenceMaxMultiSampling, value.toInt());
    }
    else if (key == "max_multi_sampling_color") {
        setInt(MMDAI::kPreferenceMaxMultiSamplingColor, value.toInt());
    }
    else if (key == "motion_adjust_frame") {
        setInt(MMDAI::kPreferenceMotionAdjustFrame, value.toInt());
    }
    else if (key == "bullet_fps") {
        setInt(MMDAI::kPreferenceBulletFPS, value.toInt());
    }
    else if (key == "rotate_step") {
        setFloat(MMDAI::kPreferenceRotateStep, value.toFloat());
    }
    else if (key == "translate_step") {
        setFloat(MMDAI::kPreferenceTranslateStep, value.toFloat());
    }
    else if (key == "distance_step") {
        setFloat(MMDAI::kPreferenceDistanceStep, value.toFloat());
    }
    else if (key == "fovy_step") {
        setFloat(MMDAI::kPreferenceFovyStep, value.toFloat());
    }
    else if (key == "use_shadow_mapping") {
        setBool(MMDAI::kPreferenceUseShadowMapping, value.toLower() == "true");
    }
    else if (key == "shadow_mapping_texture_size") {
        setInt(MMDAI::kPreferenceShadowMappingTextureSize, value.toInt());
    }
    else if (key == "shadow_mapping_self_density") {
        setFloat(MMDAI::kPreferenceShadowMappingSelfDensity, value.toFloat());
    }
    else if (key == "shadow_mapping_floor_density") {
        setFloat(MMDAI::kPreferenceShadowMappingFloorDensity, value.toFloat());
    }
    else if (key == "shadow_mapping_light_first") {
        setBool(MMDAI::kPreferenceShadowMappingLightFirst, value.toLower() == "true");
    }
    else if (key == "max_num_model") {
        setInt(MMDAI::kPreferenceMaxModelSize, value.toInt());
    }
    else if (key == "no_compatible_mode") {
        setBool(MMDAI::kPreferenceNoCompatibleMode, value.toLower() == "true");
    }
    else {
        MMDAILogWarn("unknown key %s: %s", key.toUtf8().constData(), value.toUtf8().constData());
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
    case MMDAI::kPreferenceNoCompatibleMode:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateFloatKey(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceCartoonEdgeStep:
    case MMDAI::kPreferenceCartoonEdgeWidth:
    case MMDAI::kPreferenceLogScale:
    case MMDAI::kPreferenceLightIntensity:
    case MMDAI::kPreferenceRotateStep:
    case MMDAI::kPreferenceDistanceStep:
    case MMDAI::kPreferenceFovyStep:
    case MMDAI::kPreferenceShadowMappingFloorDensity:
    case MMDAI::kPreferenceShadowMappingSelfDensity:
    case MMDAI::kPreferenceTranslateStep:
    case MMDAI::kPreferenceCameraDistance:
    case MMDAI::kPreferenceCameraFovy:
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
    case MMDAI::kPreferenceLogPosition:
    case MMDAI::kPreferenceStageSize:
    case MMDAI::kPreferenceCameraRotation:
    case MMDAI::kPreferenceCameraTransition:
        return true;
    default:
        return false;
    }
}

bool QMAPreference::validateFloat4Key(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceLightDirection:
    case MMDAI::kPreferenceCartoonEdgeSelectedColor:
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
    case MMDAI::kPreferenceMaxMultiSamplingColor:
    case MMDAI::kPreferenceMotionAdjustFrame:
    case MMDAI::kPreferenceMaxModelSize:
        return true;
    default:
        return false;
    }
}

QVariant QMAPreference::getDefaultValue(const MMDAI::PreferenceKeys key)
{
    switch (key) {
    case MMDAI::kPreferenceBulletFPS:
        return 120;
    case MMDAI::kPreferenceCampusColor:
        return QVector3D(0.0f, 0.0f, 0.0f);
    case MMDAI::kPreferenceCartoonEdgeStep:
        return 1.2f;
    case MMDAI::kPreferenceCartoonEdgeWidth:
        return 0.7f;
    case MMDAI::kPreferenceFPSPosition:
        return QVector3D(-2.5f, 22.0f, 3.0f);
    case MMDAI::kPreferenceFullScreen:
        return false;
    case MMDAI::kPreferenceLightColor:
        return QVector3D(1.0f, 1.0f, 1.0f);
    case MMDAI::kPreferenceLightDirection:
        return QVector4D(0.5f, 1.0f, 0.5f, 0.0f);
    case MMDAI::kPreferenceLightIntensity:
        return 0.6f;
    case MMDAI::kPreferenceLogPosition:
        return QVector3D(-17.5f, 3.0f, -15.0f);
    case MMDAI::kPreferenceLogScale:
        return 1.0f;
    case MMDAI::kPreferenceLogSize:
        return -1;
    case MMDAI::kPreferenceMaxMultiSampling:
        return 4;
    case MMDAI::kPreferenceMaxMultiSamplingColor:
        return 4;
    case MMDAI::kPreferenceMotionAdjustFrame:
        return 0;
    case MMDAI::kPreferenceRotateStep:
        return 0.08f;
    case MMDAI::kPreferenceDistanceStep:
        return 4.0f;
    case MMDAI::kPreferenceFovyStep:
        return 1.0f;
    case MMDAI::kPreferenceShadowMappingFloorDensity:
        return 0.5f;
    case MMDAI::kPreferenceShadowMappingLightFirst:
        return true;
    case MMDAI::kPreferenceShadowMappingSelfDensity:
        return 1.0f;
    case MMDAI::kPreferenceShadowMappingTextureSize:
        return 1024;
    case MMDAI::kPreferenceShowFPS:
        return true;
    case MMDAI::kPreferenceStageSize:
        return QVector3D(25.0f, 25.0f, 40.0f);
    case MMDAI::kPreferenceTopMost:
        return false;
    case MMDAI::kPreferenceTranslateStep:
        return 0.5f;
    case MMDAI::kPreferenceUseCartoonRendering:
        return true;
    case MMDAI::kPreferenceUseMMDLikeCartoon:
        return true;
    case MMDAI::kPreferenceUseShadowMapping:
        return false;
    case MMDAI::kPreferenceWindowSize:
        return -1;
    case MMDAI::kPreferenceCartoonEdgeSelectedColor:
        return QVector4D(1.0f, 0.0f, 0.0f, 1.0f);
    case MMDAI::kPreferenceCameraRotation:
        return QVector3D(0.0f, 0.0f, 0.0f);
    case MMDAI::kPreferenceCameraTransition:
        return QVector3D(0.0f, 13.0f, 0.0f);
    case MMDAI::kPreferenceCameraDistance:
        return 100.0f;
    case MMDAI::kPreferenceCameraFovy:
        return 16.0f;
    case MMDAI::kPreferenceMaxModelSize:
        return 10;
    case MMDAI::kPreferenceNoCompatibleMode:
        return false;
    default:
        MMDAILogWarn("should not reach here: %d", key);
        return -1;
    }
}

void QMAPreference::round(const MMDAI::PreferenceKeys key, QVariant &value)
{
    QVector3D vec3;
    QVector4D vec4;
    qreal min = 0, max = 0;
    switch (key) {
    case MMDAI::kPreferenceBulletFPS:
        value.setValue(qMin(qMax(value.toInt(), 1), 120));
        break;
    case MMDAI::kPreferenceCampusColor:
        vec3 = value.value<QVector3D>();
        min = 0.0;
        max = 1.0;
        vec3.setX(qMin(qMax(vec3.x(), min), max));
        vec3.setY(qMin(qMax(vec3.y(), min), max));
        vec3.setZ(qMin(qMax(vec3.z(), min), max));
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
        min = 0.0;
        max = 1.0;
        vec3.setX(qMin(qMax(vec3.x(), min), max));
        vec3.setY(qMin(qMax(vec3.y(), min), max));
        vec3.setZ(qMin(qMax(vec3.z(), min), max));
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
    case MMDAI::kPreferenceDistanceStep:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceFovyStep:
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
        vec3 = value.value<QVector3D>();
        min = 0.001;
        max = 1000.0;
        vec3.setX(qMin(qMax(vec3.x(), min), max));
        vec3.setY(qMin(qMax(vec3.y(), min), max));
        vec3.setZ(qMin(qMax(vec3.z(), min), max));
        value.setValue(vec3);
        break;
    case MMDAI::kPreferenceTranslateStep:
        value.setValue(qMin(qMax(value.toFloat(), 0.001f), 1000.0f));
        break;
    case MMDAI::kPreferenceWindowSize:
        // TODO
        break;
    case MMDAI::kPreferenceCartoonEdgeSelectedColor:
        vec4 = value.value<QVector4D>();
        min = 0.0;
        max = 1.0;
        vec4.setX(qMin(qMax(vec4.x(), min), max));
        vec4.setY(qMin(qMax(vec4.y(), min), max));
        vec4.setZ(qMin(qMax(vec4.z(), min), max));
        vec4.setW(qMin(qMax(vec4.w(), min), max));
        value.setValue(vec4);
        break;
    case MMDAI::kPreferenceCameraRotation:
        vec3 = value.value<QVector3D>();
        min = 0.001;
        max = 1000.0;
        vec3.setX(qMin(qMax(vec3.x(), min), max));
        vec3.setY(qMin(qMax(vec3.y(), min), max));
        vec3.setZ(qMin(qMax(vec3.z(), min), max));
        value.setValue(vec3);
        break;
    case MMDAI::kPreferenceCameraTransition:
        vec3 = value.value<QVector3D>();
        min = -10000.0;
        max = 10000.0;
        vec3.setX(qMin(qMax(vec3.x(), min), max));
        vec3.setY(qMin(qMax(vec3.y(), min), max));
        vec3.setZ(qMin(qMax(vec3.z(), min), max));
        break;
    case MMDAI::kPreferenceCameraDistance:
        value.setValue(qMin(qMax(value.toFloat(), 0.0f), 100000.0f));
        break;
    case MMDAI::kPreferenceCameraFovy:
        value.setValue(qMin(qMax(value.toFloat(), 0.0f), 180.0f));
        break;
    case MMDAI::kPreferenceMaxModelSize:
        value.setValue(qMin(qMax(value.toInt(), 1), 1024));
        break;
    default:
        break;
    }
}
