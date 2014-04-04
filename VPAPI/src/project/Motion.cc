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
#include <project/Project.h>
#include <project/Motion.h>
#include <project/BoneKeyframe.h>
#include <project/CameraKeyframe.h>
#include <project/EffectKeyframe.h>
#include <project/LightKeyframe.h>
#include <project/ModelKeyframe.h>
#include <project/MorphKeyframe.h>
#include <project/ProjectKeyframe.h>

#include <QSqlQuery>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

namespace {

#include "queries/create_project_tables.h"
#include "queries/drop_project_tables.h"
#include "queries/insert_bone_keyframe.h"
#include "queries/insert_camera_keyframe.h"
#include "queries/insert_effect_keyframe.h"
#include "queries/insert_light_keyframe.h"
#include "queries/insert_model_keyframe.h"
#include "queries/insert_morph_keyframe.h"
#include "queries/insert_project_keyframe.h"

}

namespace project
{

typedef QHash<QString, IBoneKeyframe *> BoneTrack;
typedef QMap<IKeyframe::TimeIndex, BoneTrack>  BoneTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, BoneTrackAnimation> BoneTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, ICameraKeyframe *> CameraTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, CameraTrackAnimation> CameraTrackAnimationBundle;
typedef QHash<QString, IEffectKeyframe *> EffectTrack;
typedef QMap<IKeyframe::TimeIndex, EffectTrack> EffectTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, EffectTrackAnimation> EffectTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, ILightKeyframe *> LightTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, LightTrackAnimation> LightTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, IModelKeyframe *> ModelTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, ModelTrackAnimation> ModelTrackAnimationBundle;
typedef QHash<QString, IMorphKeyframe *> MorphTrack;
typedef QMap<IKeyframe::TimeIndex, MorphTrack> MorphTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, MorphTrackAnimation> MorphTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, IProjectKeyframe *> ProjectTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, ProjectTrackAnimation> ProjectTrackAnimationBundle;

struct Motion::PrivateContext {
    PrivateContext(Project *parent, Motion *motion, int id)
        : parentProjectRef(parent),
          parentMotionRef(motion),
          rowID(id)
    {
    }
    ~PrivateContext() {
        for (BoneTrackAnimationBundle::ConstIterator it = boneBundle.begin(),
             end = boneBundle.end(); it != end; ++it) {
            for (BoneTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (CameraTrackAnimationBundle::ConstIterator it = cameraBundle.begin(),
             end = cameraBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (EffectTrackAnimationBundle::ConstIterator it = effectBundle.begin(),
             end = effectBundle.end(); it != end; ++it) {
            for (EffectTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (LightTrackAnimationBundle::ConstIterator it = lightBundle.begin(),
             end = lightBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (ModelTrackAnimationBundle::ConstIterator it = modelBundle.begin(),
             end = modelBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (MorphTrackAnimationBundle::ConstIterator it = morphBundle.begin(),
             end = morphBundle.end(); it != end; ++it) {
            for (MorphTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (ProjectTrackAnimationBundle::ConstIterator it = projectBundle.begin(),
             end = projectBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        parentProjectRef = 0;
    }

    template<typename TKeyframe, typename TTrack, typename TTrackAnimation, typename TBundle>
    TKeyframe *findKeyframeRef(const IKeyframe::TimeIndex &timeIndex, const IString *name, const IKeyframe::LayerIndex &layerIndex, const TBundle &bundle) const {
        if (bundle.contains(layerIndex)) {
            const TTrackAnimation &a = bundle.value(layerIndex);
            if (a.contains(timeIndex)) {
                const TTrack &t = a.value(timeIndex);
                const QString &n = static_cast<const String *>(name)->value();
                if (t.contains(n)) {
                    return t.value(n);
                }
            }
        }
        return 0;
    }
    template<typename TKeyframe, typename TTrackAnimation, typename TBundle>
    TKeyframe *findKeyframeRef(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex, const TBundle &bundle) {
        if (bundle.contains(layerIndex)) {
            const TTrackAnimation &a = bundle.value(layerIndex);
            if (a.contains(timeIndex)) {
                return a.value(timeIndex);
            }
        }
        return 0;
    }
    template<typename TTrack, typename TTrackAnimation, typename TKeyframe, typename TBundle>
    void removeKeyframeRef(const TKeyframe *value, TBundle &bundle) {
        const IKeyframe::LayerIndex &layerIndex = value->layerIndex();
        if (bundle.contains(layerIndex)) {
            const IKeyframe::TimeIndex &timeIndex = value->timeIndex();
            TTrackAnimation a = bundle.value(layerIndex);
            if (a.contains(timeIndex)) {
                TTrack t = a.value(timeIndex);
                const QString &n = static_cast<const String *>(value->name())->value();
                if (t.contains(n)) {
                    t.remove(n);
                }
            }
        }
    }
    template<typename TTrackAnimation, typename TKeyframe, typename TBundle>
    void removeKeyframeRef(const TKeyframe *keyframe, TBundle &bundle) {
        const IKeyframe::LayerIndex &layerIndex = keyframe->layerIndex();
        if (bundle.contains(layerIndex)) {
            const IKeyframe::TimeIndex &timeIndex = keyframe->timeIndex();
            TTrackAnimation a = bundle.value(layerIndex);
            if (a.contains(timeIndex)) {
                a.remove(timeIndex);
            }
        }
    }

    BoneKeyframe *createBoneKeyframe(const QSqlQuery &query) const {
        if (query.size() > 0) {
            int index = 1;
            QScopedPointer<BoneKeyframe> keyframe(new BoneKeyframe(parentMotionRef, query.value(0).toInt()));
            keyframe->setLayerIndex(query.value(index++).toInt());
            keyframe->setTimeIndex(query.value(index++).toInt());
            keyframe->setName(String::create(query.value(index++).toString().toStdString()));
            keyframe->setLocalTranslation(Vector3(query.value(index++).toFloat(),  query.value(index++).toFloat(), query.value(index++).toFloat()));
            keyframe->setLocalOrientation(Quaternion(query.value(index++).toFloat(), query.value(index++).toFloat(), query.value(index++).toFloat(), query.value(index++).toFloat()));
            return keyframe.take();
        }
        return 0;
    }
    CameraKeyframe *createCameraKeyframe(const QSqlQuery &query) const {
        if (query.size() > 0) {
            int index = 1;
            QScopedPointer<CameraKeyframe> keyframe(new CameraKeyframe(parentMotionRef, query.value(0).toInt()));
            keyframe->setLayerIndex(query.value(index++).toInt());
            keyframe->setTimeIndex(query.value(index++).toInt());
            keyframe->setLookAt(Vector3(query.value(index++).toFloat(),  query.value(index++).toFloat(), query.value(index++).toFloat()));
            keyframe->setAngle(Vector3(query.value(index++).toFloat(), query.value(index++).toFloat(), query.value(index++).toFloat()));
            keyframe->setFov(query.value(index++).toFloat());
            keyframe->setDistance(query.value(index++).toFloat());
            return keyframe.take();
        }
        return 0;
    }
    LightKeyframe *createLightKeyframe(const QSqlQuery &query) const {
        if (query.size() > 0) {
            int index = 1;
            QScopedPointer<LightKeyframe> keyframe(new LightKeyframe(parentMotionRef, query.value(0).toInt()));
            keyframe->setLayerIndex(query.value(index++).toInt());
            keyframe->setTimeIndex(query.value(index++).toInt());
            keyframe->setColor(Vector3(query.value(index++).toFloat(),  query.value(index++).toFloat(), query.value(index++).toFloat()));
            keyframe->setDirection(Vector3(query.value(index++).toFloat(), query.value(index++).toFloat(), query.value(index++).toFloat()));
            return keyframe.take();
        }
        return 0;
    }
    MorphKeyframe *createMorphProject(const QSqlQuery &query) const {
        if (query.size() > 0) {
            int index = 1;
            QScopedPointer<MorphKeyframe> keyframe(new MorphKeyframe(parentMotionRef, query.value(0).toInt()));
            keyframe->setLayerIndex(query.value(index++).toInt());
            keyframe->setTimeIndex(query.value(index++).toInt());
            keyframe->setName(String::create(query.value(index++).toString().toStdString()));
            keyframe->setWeight(query.value(index++).toFloat());
            return keyframe.take();
        }
        return 0;
    }


    Project *parentProjectRef;
    Motion *parentMotionRef;
    mutable BoneTrackAnimationBundle boneBundle;
    mutable CameraTrackAnimationBundle cameraBundle;
    mutable EffectTrackAnimationBundle effectBundle;
    mutable LightTrackAnimationBundle lightBundle;
    mutable ModelTrackAnimationBundle modelBundle;
    mutable MorphTrackAnimationBundle morphBundle;
    mutable ProjectTrackAnimationBundle projectBundle;
    int rowID;
};

Motion::Motion(Project *parent, int rowID)
    : m_context(new PrivateContext(parent, this, rowID))
{
}

Motion::~Motion()
{
    delete m_context;
    m_context = 0;
}

bool Motion::load(const uint8 * /* data */, vsize /* size */)
{
    return false;
}

void Motion::save(uint8 * /* data */) const
{
}

vsize Motion::estimateSize() const
{
    return 0;
}

IMotion::Error Motion::error() const
{
    return kNoError;
}

IModel *Motion::parentModelRef() const
{
    return 0;
}

void Motion::refresh()
{
}

void Motion::seekSeconds(const float64 &seconds)
{
}

void Motion::seekSceneSeconds(const float64 &seconds, Scene *scene, int flags)
{
}

void Motion::seekTimeIndex(const IKeyframe::TimeIndex &timeIndex)
{
}

void Motion::seekSceneTimeIndex(const IKeyframe::TimeIndex &timeIndex, Scene *scene, int flags)
{
}

void Motion::reset()
{
}

float64 Motion::durationSeconds() const
{
    return 0;
}

IKeyframe::TimeIndex Motion::durationTimeIndex() const
{
    return 0;
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &timeIndex) const
{
    return false;
}

IBoneKeyframe *Motion::createBoneKeyframe()
{
    return new BoneKeyframe(this, -1);
}

ICameraKeyframe *Motion::createCameraKeyframe()
{
    return new CameraKeyframe(this, -1);
}

IEffectKeyframe *Motion::createEffectKeyframe()
{
    return new EffectKeyframe(this, -1);
}

ILightKeyframe *Motion::createLightKeyframe()
{
    return new LightKeyframe(this, -1);
}

IModelKeyframe *Motion::createModelKeyframe()
{
    return new ModelKeyframe(this, -1);
}

IMorphKeyframe *Motion::createMorphKeyframe()
{
    return new MorphKeyframe(this, -1);
}

IProjectKeyframe *Motion::createProjectKeyframe()
{
    return new ProjectKeyframe(this, -1);
}

void Motion::addKeyframe(IKeyframe *value)
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.bindValue(":parent_layer_id", value->layerIndex());
    query.bindValue(":time_index", value->timeIndex());
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe: {
        BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_bone_keyframe_sql));
        const Vector3 &t = keyframe->localTranslation();
        query.bindValue(":translation_x", t.x());
        query.bindValue(":translation_y", t.y());
        query.bindValue(":translation_z", t.z());
        const Quaternion &o = keyframe->localOrientation();
        query.bindValue(":orientation_x", o.x());
        query.bindValue(":orientation_y", o.y());
        query.bindValue(":orientation_z", o.z());
        query.bindValue(":orientation_w", o.w());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        CameraKeyframe *keyframe = reinterpret_cast<CameraKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_camera_keyframe_sql));
        const Vector3 &p = keyframe->lookAt();
        query.bindValue(":position_x", p.x());
        query.bindValue(":position_y", p.y());
        query.bindValue(":position_z", p.z());
        const Vector3 &a = keyframe->angle();
        query.bindValue(":angle_x", a.x());
        query.bindValue(":angle_y", a.y());
        query.bindValue(":angle_z", a.z());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        EffectKeyframe *keyframe = reinterpret_cast<EffectKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_effect_keyframe_sql));
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kLightKeyframe: {
        LightKeyframe *keyframe = reinterpret_cast<LightKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_light_keyframe_sql));
        const Vector3 &c = keyframe->color();
        query.bindValue(":color_r", c.x());
        query.bindValue(":color_g", c.y());
        query.bindValue(":color_b", c.z());
        const Vector3 &d = keyframe->direction();
        query.bindValue(":direction_x", d.x());
        query.bindValue(":direction_y", d.y());
        query.bindValue(":direction_z", d.z());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kModelKeyframe: {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_model_keyframe_sql));
        query.bindValue(":edge_width", keyframe->edgeWidth());
        const Vector3 &c = keyframe->edgeColor();
        query.bindValue(":edge_color_r", c.x());
        query.bindValue(":edge_color_g", c.y());
        query.bindValue(":edge_color_b", c.z());
        query.bindValue(":edge_color_a", c.w());
        query.bindValue(":is_visible", keyframe->isVisible());
        query.bindValue(":is_shadow_enabled", keyframe->isShadowEnabled());
        query.bindValue(":is_add_blend_enabled", keyframe->isAddBlendEnabled());
        query.bindValue(":is_physics_enabled", keyframe->isPhysicsEnabled());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        MorphKeyframe *keyframe = reinterpret_cast<MorphKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_morph_keyframe_sql));
        query.bindValue(":weight", keyframe->weight());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        ProjectKeyframe *keyframe = reinterpret_cast<ProjectKeyframe *>(value);
        query.prepare(reinterpret_cast<const char *>(g_insert_project_keyframe_sql));
        query.bindValue(":shadow_mode", keyframe->shadowMode());
        query.bindValue(":shadow_distance", keyframe->shadowDistance());
        query.bindValue(":gravity_factor", keyframe->gravityFactor());
        const Vector3 &d = keyframe->gravityDirection();
        query.bindValue(":gravity_direction_x", d.x());
        query.bindValue(":gravity_direction_y", d.y());
        query.bindValue(":gravity_direction_z", d.z());
        query.exec();
        keyframe->setRowID(query.lastInsertId().toInt());
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    QString tablePrefix;
    switch (value) {
    case IKeyframe::kBoneKeyframe: {
        tablePrefix = QStringLiteral("bone");
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        tablePrefix = QStringLiteral("camera");
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        tablePrefix = QStringLiteral("effect");
        break;
    }
    case IKeyframe::kLightKeyframe: {
        tablePrefix = QStringLiteral("light");
        break;
    }
    case IKeyframe::kModelKeyframe: {
        tablePrefix = QStringLiteral("model");
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        tablePrefix = QStringLiteral("morph");
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        tablePrefix = QStringLiteral("project");
        break;
    }
    default:
        break;
    }
    query.prepare(QStringLiteral("select count(*) from %1_keyframes join %1_layers on %1_layers.id = %1_keyframes.parent_layer_id where %1_layers.parent_motion_id = :parent_motion_id").arg(tablePrefix));
    query.bindValue(":parent_motion_id", m_context->rowID);
    query.exec();
    return query.value(0).toInt();
}

IKeyframe::LayerIndex Motion::countLayers(const IString * /* name */, IKeyframe::Type type) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    QString tablePrefix;
    switch (type) {
    case IKeyframe::kBoneKeyframe: {
        query.prepare(QStringLiteral("select count(distinct id) from camera_layers "
                                     "where camera_layers.parent_motion_id = :parent_motion_id"));
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        query.prepare(QStringLiteral("select count(distinct id) from camera_layers where camera_layers.parent_motion_id = :parent_motion_id"));
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        tablePrefix = QStringLiteral("effect");
        break;
    }
    case IKeyframe::kLightKeyframe: {
        query.prepare(QStringLiteral("select count(distinct id) from camera_layers where camera_layers.parent_motion_id = :parent_motion_id"));
        break;
    }
    case IKeyframe::kModelKeyframe: {
        query.prepare(QStringLiteral("select count(distinct id) from model_layers where model_layers.parent_motion_id = :parent_motion_id"));
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        tablePrefix = QStringLiteral("morph");
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        query.prepare(QStringLiteral("select count(distinct id) from project_layers where project_layers.parent_motion_id = :parent_motion_id"));
        break;
    }
    default:
        break;
    }
    query.bindValue(":parent_motion_id", m_context->rowID);
    query.exec();
    return query.value(0).toInt();
}

void Motion::getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                             const IKeyframe::LayerIndex &layerIndex,
                             IKeyframe::Type type,
                             Array<IKeyframe *> &keyframes)
{
}

IBoneKeyframe *Motion::findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                           const IString *name,
                                           const IKeyframe::LayerIndex &layerIndex) const
{
    if (IBoneKeyframe *keyframe = m_context->findKeyframeRef<IBoneKeyframe, BoneTrack, BoneTrackAnimation>(timeIndex, name, layerIndex, m_context->boneBundle)) {
        return keyframe;
    }
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.prepare("select k.id, k.parent_layer_id, t.name, k.translation_x, k.translation_y, k.translation_z, k.orientation_x, k.orientation_y, k.orientation_z, k.orientation_w from bone_keyframes as k join bone_tracks as t on t.id = k.parent_track.id join bone_layers as l on l.id = k.parent_layer_id where k.time_index = :time_index and t.name = :name and l.index = :layer_index");
    query.bindValue(":time_index", timeIndex);
    query.bindValue(":name", static_cast<const String *>(name)->value());
    query.bindValue(":layer_index", layerIndex);
    query.exec();
    return 0; //m_context->addBoneKeyframe(query);
}

IBoneKeyframe *Motion::findBoneKeyframeRefAt(int index) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.prepare("select k.id, l.index, t.name, k.translation_x, k.translation_y, k.translation_z, k.orientation_x, k.orientation_y, k.orientation_z, k.orientation_w from bone_keyframes as k join bone_tracks as t on t.id = k.parent_track.id join bone_layers as l on l.id = k.parent_layer_id limit 1 offset :index");
    query.bindValue(":index", index);
    query.exec();
    return m_context->createBoneKeyframe(query);
}

ICameraKeyframe *Motion::findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                               const IKeyframe::LayerIndex &layerIndex) const
{
    if (ICameraKeyframe *keyframe = m_context->findKeyframeRef<ICameraKeyframe, CameraTrackAnimation>(timeIndex, layerIndex, m_context->cameraBundle)) {
        return keyframe;
    }
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframeRefAt(int index) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.prepare("select k.id, l.index, k.position_x, k.position_y, k.position_z, k.angle_x, k.angle_y, k.angle_z, k.fov, k.distance from camera_keyframes as k join camera_layers as l on l.id = k.parent_layer_id limit 1 offset :index");
    query.bindValue(":index", index);
    query.exec();
    return m_context->createCameraKeyframe(query);
}

IEffectKeyframe *Motion::findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                               const IString *name,
                                               const IKeyframe::LayerIndex &layerIndex) const
{
    if (IEffectKeyframe *keyframe = m_context->findKeyframeRef<IEffectKeyframe, EffectTrack, EffectTrackAnimation>(timeIndex, name, layerIndex, m_context->effectBundle)) {
        return keyframe;
    }
    return 0;
}

IEffectKeyframe *Motion::findEffectKeyframeRefAt(int index) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    if (ILightKeyframe *keyframe = m_context->findKeyframeRef<ILightKeyframe, LightTrackAnimation>(timeIndex, layerIndex, m_context->lightBundle)) {
        return keyframe;
    }
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRefAt(int index) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.prepare("select k.id, l.index, k.color_r, k.color_g, k.color_b, k.direction_x, k.direction_y, k.direction_z from light_keyframes as k join light_layers as l on l.id = k.parent_layer_id limit 1 offset :index");
    query.bindValue(":index", index);
    query.exec();
    return m_context->createLightKeyframe(query);
}

IModelKeyframe *Motion::findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    if (IModelKeyframe *keyframe = m_context->findKeyframeRef<IModelKeyframe, ModelTrackAnimation>(timeIndex, layerIndex, m_context->modelBundle)) {
        return keyframe;
    }
    return 0;
}

IModelKeyframe *Motion::findModelKeyframeRefAt(int index) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IString *name,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    if (IMorphKeyframe *keyframe = m_context->findKeyframeRef<IMorphKeyframe, MorphTrack, MorphTrackAnimation>(timeIndex, name, layerIndex, m_context->morphBundle)) {
        return keyframe;
    }
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeRefAt(int index) const
{
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    query.prepare("select k.id, l.index, t.name, k.weight from morph_keyframes as k join morph_tracks as t on t.id = k.parent_track.id join morph_layers as l on l.id = k.parent_layer_id limit 1 offset :index");
    query.bindValue(":index", index);
    query.exec();
    return m_context->createMorphProject(query);
}

IProjectKeyframe *Motion::findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                 const IKeyframe::LayerIndex &layerIndex) const
{
    if (m_context->projectBundle.contains(layerIndex)) {
        const ProjectTrackAnimation &a = m_context->projectBundle.value(timeIndex);
        if (a.contains(timeIndex)) {
            return a.value(timeIndex);
        }
    }
    return 0;
}

IProjectKeyframe *Motion::findProjectKeyframeRefAt(int index) const
{
    return 0;
}

void Motion::replaceKeyframe(IKeyframe * /* value */, bool /* alsoDelete */)
{
}

void Motion::removeKeyframe(IKeyframe *value)
{
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe: {
        IBoneKeyframe *keyframe = static_cast<IBoneKeyframe *>(value);
        m_context->removeKeyframeRef<BoneTrack, BoneTrackAnimation>(keyframe, m_context->boneBundle);
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        ICameraKeyframe *keyframe = static_cast<ICameraKeyframe *>(value);
        m_context->removeKeyframeRef<CameraTrackAnimation>(keyframe, m_context->cameraBundle);
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        IEffectKeyframe *keyframe = static_cast<IEffectKeyframe *>(value);
        m_context->removeKeyframeRef<EffectTrack, EffectTrackAnimation>(keyframe, m_context->effectBundle);
        break;
    }
    case IKeyframe::kLightKeyframe: {
        ILightKeyframe *keyframe = static_cast<ILightKeyframe *>(value);
        m_context->removeKeyframeRef<LightTrackAnimation>(keyframe, m_context->lightBundle);
        break;
    }
    case IKeyframe::kModelKeyframe: {
        IModelKeyframe *keyframe = static_cast<IModelKeyframe *>(value);
        m_context->removeKeyframeRef<ModelTrackAnimation>(keyframe, m_context->modelBundle);
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        IMorphKeyframe *keyframe = static_cast<IMorphKeyframe *>(value);
        m_context->removeKeyframeRef<MorphTrack, MorphTrackAnimation>(keyframe, m_context->morphBundle);
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        IProjectKeyframe *keyframe = static_cast<IProjectKeyframe *>(value);
        m_context->removeKeyframeRef<ProjectTrackAnimation>(keyframe, m_context->projectBundle);
        break;
    }
    default:
        break;
    }
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    removeKeyframe(value);
    QSqlQuery query(*m_context->parentProjectRef->databaseHandle());
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe: {
        query.prepare("delete from bone_keyframes where id = :id");
        query.bindValue(":id", static_cast<BoneKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        query.prepare("delete from camera_keyframes where id = :id");
        query.bindValue(":id", static_cast<CameraKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        query.prepare("delete from effect_keyframes where id = :id");
        query.bindValue(":id", static_cast<EffectKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kLightKeyframe: {
        query.prepare("delete from light_keyframes where id = :id");
        query.bindValue(":id", static_cast<LightKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kModelKeyframe: {
        query.prepare("delete from model_keyframes where id = :id");
        query.bindValue(":id", static_cast<ModelKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        query.prepare("delete from morph_keyframes where id = :id");
        query.bindValue(":id", static_cast<MorphKeyframe *>(value)->rowID());
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        query.prepare("delete from project_keyframes where id = :id");
        query.bindValue(":id", static_cast<ProjectKeyframe *>(value)->rowID());
        break;
    }
    default:
        break;
    }
    if (query.exec()) {
        delete value;
        value = 0;
    }
}

void Motion::update(IKeyframe::Type /* type */)
{
}

void Motion::getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBoneKeyframe: {
        for (BoneTrackAnimationBundle::ConstIterator it = m_context->boneBundle.begin(),
             end = m_context->boneBundle.end(); it != end; ++it) {
            for (BoneTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                for (BoneTrack::ConstIterator it3 = it2.value().begin(),
                     end3 = it2.value().end(); it3 != end3; ++it3) {
                    value.append(it3.value());
                }
            }
        }
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        for (CameraTrackAnimationBundle::ConstIterator it = m_context->cameraBundle.begin(),
             end = m_context->cameraBundle.end(); it != end; ++it) {
            for (CameraTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                value.append(it2.value());
            }
        }
        break;
    }
    case IKeyframe::kLightKeyframe: {
        for (LightTrackAnimationBundle::ConstIterator it = m_context->lightBundle.begin(),
             end = m_context->lightBundle.end(); it != end; ++it) {
            for (LightTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                value.append(it2.value());
            }
        }
        break;
    }
    case IKeyframe::kModelKeyframe: {
        for (ModelTrackAnimationBundle::ConstIterator it = m_context->modelBundle.begin(),
             end = m_context->modelBundle.end(); it != end; ++it) {
            for (ModelTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                value.append(it2.value());
            }
        }
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        for (EffectTrackAnimationBundle::ConstIterator it = m_context->effectBundle.begin(),
             end = m_context->effectBundle.end(); it != end; ++it) {
            for (EffectTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                for (EffectTrack::ConstIterator it3 = it2.value().begin(),
                     end3 = it2.value().end(); it3 != end3; ++it3) {
                    value.append(it3.value());
                }
            }
        }
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        for (ProjectTrackAnimationBundle::ConstIterator it = m_context->projectBundle.begin(),
             end = m_context->projectBundle.end(); it != end; ++it) {
            for (ProjectTrackAnimation::ConstIterator it2 = it.value().begin(),
                 end2 = it.value().end(); it2 != end2; ++it2) {
                value.append(it2.value());
            }
        }
        break;
    }
    default:
        break;
    }
}

void Motion::setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        break;
    case IKeyframe::kCameraKeyframe:
        break;
    case IKeyframe::kLightKeyframe:
        break;
    case IKeyframe::kModelKeyframe:
        break;
    case IKeyframe::kMorphKeyframe:
        break;
    case IKeyframe::kProjectKeyframe:
        break;
    default:
        break;
    }
}

void Motion::createFirstKeyframesUnlessFound()
{
}

IMotion *Motion::clone() const
{
    return 0;
}

Scene *Motion::parentSceneRef() const
{
    return 0;
}

const IString *Motion::name() const
{
    return 0;
}

IMotion::FormatType Motion::type() const
{
    return kMVDFormat;
}

int Motion::rowID() const
{
    return m_context->rowID;
}

} /* namespace project */
