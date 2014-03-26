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

#include <QtCore>
#include <QtSql>
#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/Encoding.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

namespace {

static QString slurp(const QString &filename)
{
    QFile file(filename);
    return file.open(QFile::ReadOnly) ? file.readAll() : QString();
}

static inline QString to_s(const IString *s)
{
    return s ? QString(reinterpret_cast<const char *>(s->toByteArray())) : QStringLiteral("");
}

static void createTables()
{
    QSqlQuery query;
    if (!query.exec("pragma foreign_keys = on;")) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_models_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_vertices_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_bones_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_ik_constraints_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_ik_joints_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_materials_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_labels_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_morphs_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_rigidbodies_table.sql"))) {
        qWarning() << query.lastError();
    }
    if (!query.exec(slurp(":queries/create_joints_table.sql"))) {
        qWarning() << query.lastError();
    }
}

static int addModel(const IModel *model, const QString &filename, const QByteArray &sha1Hash)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_model_record.sql"));
    query.bindValue(":version", model->version());
    query.bindValue(":encoding", int(model->encodingType()));
    query.bindValue(":uv", model->maxUVCount());
    query.bindValue(":name_ja", to_s(model->name(IEncoding::kJapanese)));
    query.bindValue(":name_en", to_s(model->name(IEncoding::kEnglish)));
    query.bindValue(":comment_ja", to_s(model->comment(IEncoding::kJapanese)));
    query.bindValue(":comment_en", to_s(model->comment(IEncoding::kEnglish)));
    query.bindValue(":filename", filename);
    query.bindValue(":sha1", sha1Hash);
    if (!query.exec()) {
        qWarning() << query.lastError();
        return -1;
    }
    return query.lastInsertId().toInt();
}

static void importVertices(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_vertex_record.sql"));
    Array<IVertex *> vertices;
    model->getVertexRefs(vertices);
    const int nvertices = vertices.count();
    for (int i = 0; i < nvertices; i++) {
        IVertex *vertex = vertices[i];
        query.bindValue(":index", vertex->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":type", int(vertex->type()));
        if (!query.exec()) {
            qWarning() << query.lastError() << "at vertex" << i << "on model" << modelID;
        }
    }
}

static void importBones(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_bone_record.sql"));
    Array<IBone *> bones;
    model->getBoneRefs(bones);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        const IBone *bone = bones[i];
        query.bindValue(":index", bone->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(bone->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(bone->name(IEncoding::kEnglish)));
        const IBone *parent = bone->parentBoneRef();
        query.bindValue(":parent_bone", parent ? parent->index() : QVariant());
        const IBone *destination = bone->destinationOriginBoneRef();
        query.bindValue(":destination_bone", destination ? destination->index() : QVariant());
        query.bindValue(":inherent_coefficient", bone->inherentCoefficient());
        query.bindValue(":is_movable", bone->isMovable());
        query.bindValue(":is_rotateable", bone->isRotateable());
        query.bindValue(":is_visible", bone->isVisible());
        query.bindValue(":is_interactive", bone->isInteractive());
        query.bindValue(":is_inherent_translation_enabled", bone->isInherentTranslationEnabled());
        query.bindValue(":is_inherent_orientation_enabled", bone->isInherentOrientationEnabled());
        query.bindValue(":has_inverse_kinematics", bone->hasInverseKinematics());
        query.bindValue(":has_local_axes", bone->hasLocalAxes());
        query.bindValue(":has_fixed_axes", bone->hasFixedAxes());
        if (!query.exec()) {
            qWarning() << query.lastError() << "at bone" << i << "on model" << modelID;
        }
    }
}

static void importIKConstraints(const IModel *model, int modelID)
{
    QSqlQuery constraintQuery, jointQuery;
    constraintQuery.prepare(slurp(":queries/insert_ik_constraint_record.sql"));
    jointQuery.prepare(slurp(":queries/insert_ik_joint_record.sql"));
    Array<IBone::IKConstraint *> constraints;
    Array<IBone::IKJoint *> joints;
    model->getIKConstraintRefs(constraints);
    const int nbones = constraints.count();
    for (int i = 0; i < nbones; i++) {
        const IBone::IKConstraint *constraint = constraints[i];
        constraintQuery.bindValue(":parent_model", modelID);
        constraintQuery.bindValue(":effector_bone", constraint->effectorBoneRef()->index());
        if (const IBone *rootBone = constraint->rootBoneRef()) {
            constraintQuery.bindValue(":root_bone", rootBone->index());
        }
        constraintQuery.bindValue(":angle_limit", constraint->angleLimit());
        constraintQuery.bindValue(":num_iterations", constraint->numIterations());
        if (!constraintQuery.exec()) {
            qWarning() << constraintQuery.lastError() << "at constraint" << i << "on model" << modelID;
        }
        else {
            int constraintID = constraintQuery.lastInsertId().toInt();
            constraint->getJointRefs(joints);
            const int njoints = joints.count();
            for (int j = 0; j < njoints; j++) {
                const IBone::IKJoint *joint = joints[j];
                jointQuery.bindValue(":constraint", constraintID);
                jointQuery.bindValue(":has_angle_limit", joint->hasAngleLimit());
                jointQuery.bindValue(":upper_limit_x", joint->upperLimit().x());
                jointQuery.bindValue(":upper_limit_y", joint->upperLimit().y());
                jointQuery.bindValue(":upper_limit_z", joint->upperLimit().z());
                jointQuery.bindValue(":lower_limit_x", joint->lowerLimit().x());
                jointQuery.bindValue(":lower_limit_y", joint->lowerLimit().y());
                jointQuery.bindValue(":lower_limit_z", joint->lowerLimit().z());
                if (!jointQuery.exec()) {
                    qWarning() << jointQuery.lastError() << "at joint" << j << "of constraint" << i << "on model" << modelID;
                }
            }
        }
    }
}

static void importMaterials(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_material_record.sql"));
    Array<IMaterial *> materials;
    model->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        query.bindValue(":index", material->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(material->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(material->name(IEncoding::kEnglish)));
        query.bindValue(":edge_size", material->edgeSize());
        query.bindValue(":is_casting_shadow_enabled", material->isCastingShadowEnabled());
        query.bindValue(":is_casting_shadow_map_enabled", material->isCastingShadowMapEnabled());
        query.bindValue(":is_culling_disabled", material->isCullingDisabled());
        query.bindValue(":is_edge_enabled", material->isEdgeEnabled());
        query.bindValue(":is_shadow_map_enabled", material->isShadowMapEnabled());
        query.bindValue(":is_shared_toon_texture_used", material->isSharedToonTextureUsed());
        query.bindValue(":is_vertex_color_enabled", material->isVertexColorEnabled());
        query.bindValue(":main_texture_path", to_s(material->mainTexture()));
        query.bindValue(":sphere_texture_path", to_s(material->sphereTexture()));
        query.bindValue(":toon_texture_path", to_s(material->toonTexture()));
        query.bindValue(":user_data", to_s(material->userDataArea()));
        query.bindValue(":index_range_count", material->indexRange().count);
        if (!query.exec()) {
            qWarning() << query.lastError() << "at material" << i << "on model" << modelID;
        }
    }
}

static void importLabels(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_label_record.sql"));
    Array<ILabel *> labels;
    model->getLabelRefs(labels);
    const int nlabels = labels.count();
    for (int i = 0; i < nlabels; i++) {
        const ILabel *label = labels[i];
        query.bindValue(":index", label->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(label->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(label->name(IEncoding::kEnglish)));
        query.bindValue(":is_special", label->isSpecial());
        if (!query.exec()) {
            qWarning() << query.lastError() << "at label" << i << "on model" << modelID;
        }
    }
}

static void importMorphs(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_morph_record.sql"));
    Array<IMorph *> morphs;
    model->getMorphRefs(morphs);
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        const IMorph *morph = morphs[i];
        query.bindValue(":index", morph->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(morph->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(morph->name(IEncoding::kEnglish)));
        query.bindValue(":category", int(morph->category()));
        query.bindValue(":type", int(morph->type()));
        if (!query.exec()) {
            qWarning() << query.lastError() << "at morph" << i << "on model" << modelID;
        }
    }
}

static void importRigidBodies(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_rigidbody_record.sql"));
    Array<IRigidBody *> rigidBodies;
    model->getRigidBodyRefs(rigidBodies);
    const int nbodies = rigidBodies.count();
    for (int i = 0; i < nbodies; i++) {
        const IRigidBody *body = rigidBodies[i];
        query.bindValue(":index", body->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(body->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(body->name(IEncoding::kEnglish)));
        query.bindValue(":object_type", body->objectType());
        query.bindValue(":shape_type", body->shapeType());
        query.bindValue(":mass", body->mass());
        query.bindValue(":linear_damping", body->linearDamping());
        query.bindValue(":angular_damping", body->angularDamping());
        query.bindValue(":friction", body->friction());
        query.bindValue(":restitution", body->restitution());
        if (!query.exec()) {
            qWarning() << query.lastError() << "at body" << i << "on model" << modelID;
        }
    }
}

static void importJoints(const IModel *model, int modelID)
{
    QSqlQuery query;
    query.prepare(slurp(":queries/insert_joint_record.sql"));
    Array<IJoint *> joints;
    model->getJointRefs(joints);
    const int njoints = joints.count();
    for (int i = 0; i < njoints; i++) {
        const IJoint *joint = joints[i];
        query.bindValue(":index", joint->index());
        query.bindValue(":parent_model", modelID);
        query.bindValue(":name_ja", to_s(joint->name(IEncoding::kJapanese)));
        query.bindValue(":name_en", to_s(joint->name(IEncoding::kEnglish)));
        query.bindValue(":type", joint->type());
        if (!query.exec()) {
            qWarning() << query.lastError() << "at joint" << i << "on model" << modelID;
        }
    }
}

static void findFiles(const QString &basePath, QStringList &result)
{
    QDir dir(basePath);
    foreach (const QString &f, dir.entryList(QStringList() << "*.pmd" << "*.pmx", QDir::Files)) {
        result << dir.absoluteFilePath(f);
    }
    foreach (const QString &d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        findFiles(dir.absoluteFilePath(d), result);
    }
}

} /* namespace anonymous */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Encoding::Dictionary dict;
    Encoding encoding(&dict);
    Factory factory(&encoding);

    QFile filePath(QDir::home().absoluteFilePath("test.db"));
    filePath.remove();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath.fileName());
    if (db.open()) {
        QFileInfo finfo;
        createTables();
        QStringList files;
        findFiles(a.arguments().at(1), files);
        foreach (const QString &s, files) {
            finfo.setFile(s);
            if (finfo.exists()) {
                if (finfo.suffix() == "pmx") { // || finfo.suffix() == "pmd") {
                    QFile file(finfo.absoluteFilePath());
                    if (file.open(QFile::ReadOnly)) {
                        const QByteArray bytes = file.readAll();
                        const uint8 *ptr = reinterpret_cast<const uint8 *>(bytes.constData());
                        bool ok = false;
                        IModel *model = factory.createModel(ptr, bytes.size(), ok);
                        if (ok) {
                            db.transaction();
                            int modelID = addModel(model, finfo.fileName(), QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex());
                            if (modelID >= 0) {
                                importVertices(model, modelID);
                                importBones(model, modelID);
                                importIKConstraints(model, modelID);
                                importMaterials(model, modelID);
                                importLabels(model, modelID);
                                importMorphs(model, modelID);
                                importRigidBodies(model, modelID);
                                importJoints(model, modelID);
                                if (!db.commit()) {
                                    qWarning() << "Cannot commit database:" << db.lastError();
                                    if (!db.rollback()) {
                                        qFatal("Cannot rollback database: %s", qPrintable(db.lastError().text()));
                                    }
                                }
                            }
                            else if (!db.rollback()) {
                                qFatal("Cannot rollback database: %s", qPrintable(db.lastError().text()));
                            }
                        }
                    }
                }
                else if (finfo.suffix() == "vmd" || finfo.suffix() == "mvd") {
                    QFile file(finfo.absoluteFilePath());
                    if (file.open(QFile::ReadOnly)) {
                        const QByteArray bytes = file.readAll();
                        const uint8 *ptr = reinterpret_cast<const uint8 *>(bytes.constData());
                        bool ok = false;
                        IMotion *motion = factory.createMotion(ptr, bytes.size(), 0, ok);
                        if (ok && motion) {
                        }
                    }
                }
            }
        }
    }
    else {
        qWarning() << db.lastError();
    }

    return 0;
}
