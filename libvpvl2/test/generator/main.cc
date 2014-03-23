#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/BaseApplicationContext.h> /* BaseApplicationContext::initializeOnce */
#include <vpvl2/extensions/icu4c/Encoding.h>

#include <stdio.h>
#include <set>

/* libvsq */
#include "FileInputStream.hpp"
#include "VSQFileReader.hpp"

#include <QtCore>
#include <QtMath>
#include <QXmlStreamReader>

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;

using namespace VSQ_NS;

namespace {

static void AssignVertex(IVertex *vertex, IVertex::Type type)
{
    vertex->setOrigin(Vector3(0.1, 0.2, 0.3));
    vertex->setNormal(Vector3(0.4, 0.5, 0.6));
    vertex->setTextureCoord(Vector3(0.7, 0.8, 0.9));
    vertex->setEdgeSize(0.42);
    vertex->setType(type);
}

static void AssignMaterial(IMaterial *material, int flags)
{
    String n1("Japanese Material Name"), n2("English Material Name"), n3("User Area Data"),
            main("MainTexture.png"), toon("ToonTexture.png"), sphere("SphereTexture.png");
    material->setName(&n1, IEncoding::kJapanese);
    material->setName(&n2, IEncoding::kEnglish);
    material->setAmbient(Color(0.1, 0.2, 0.3, 1.0));
    material->setDiffuse(Color(0.4, 0.3, 0.2, 0.1));
    material->setEdgeColor(Color(0.4, 0.5, 0.6, 0.7));
    material->setFlags(flags);
    material->setEdgeSize(0.7);
    material->setShininess(0.8);
    material->setSpecular(Color(0.9, 0.8, 0.7, 1.0));
    material->setUserDataArea(&n3);
    material->setMainTexture(&main);
    material->setToonTexture(&toon);
    material->setSphereTexture(&sphere);
}

static void AssignLabel(ILabel *label)
{
    String n1("Japanese Label Name"), n2("English Label Name");
    label->setName(&n1, IEncoding::kJapanese);
    label->setName(&n2, IEncoding::kEnglish);
}

static void AssignMorph(IMorph *morph, IMorph::Type type)
{
    String n1("Japanese Morph Name"), n2("English Morph Name");
    morph->setName(&n1, IEncoding::kJapanese);
    morph->setName(&n2, IEncoding::kEnglish);
    morph->setWeight(0.42);
    morph->setType(type);
}

static void AssignJoint(IJoint *joint, IJoint::Type type)
{
    String n1("Japanese Joint Name"), n2("English Joint Name");
    joint->setName(&n1, IEncoding::kJapanese);
    joint->setName(&n2, IEncoding::kEnglish);
    joint->setType(type);
    joint->setPosition(Vector3(1, 2, 3));
    joint->setPositionLowerLimit(Vector3(0.1, 0.2, 0.3));
    joint->setPositionStiffness(Vector3(2, 4, 6));
    joint->setPositionUpperLimit(Vector3(7, 8, 9));
    joint->setRotation(Vector3(btRadians(15), btRadians(30), btRadians(45)));
    joint->setRotationLowerLimit(Vector3(btRadians(5), btRadians(10), btRadians(15)));
    joint->setRotationStiffness(Vector3(1, 3, 5));
    joint->setRotationUpperLimit(Vector3(btRadians(30), btRadians(60), btRadians(90)));
}

void CreateModel(IModel *model, const char *filename)
{
    IVertex *vertex = 0;
    {
        vertex = model->createVertex();
        AssignVertex(vertex, IVertex::kBdef1);
        vertex->setOriginUV(0, Vector4(1.0, 1.1, 1.2, 1.3));
        vertex->setOriginUV(1, Vector4(1.4, 1.5, 1.6, 1.7));
        vertex->setOriginUV(2, Vector4(1.8, 1.9, 2.0, 2.1));
        vertex->setOriginUV(3, Vector4(2.2, 2.3, 2.4, 2.5));
        vertex->setWeight(0, 1.0);
        model->addVertex(vertex);
        IVertex *bdef2 = model->createVertex();
        AssignVertex(bdef2, IVertex::kBdef2);
        bdef2->setWeight(0, 0.5);
        model->addVertex(bdef2);
        IVertex *bdef4 = model->createVertex();
        AssignVertex(bdef4, IVertex::kBdef4);
        bdef4->setWeight(0, 0.1);
        bdef4->setWeight(1, 0.2);
        bdef4->setWeight(2, 0.3);
        bdef4->setWeight(3, 0.4);
        model->addVertex(bdef4);
        IVertex *qdef = model->createVertex();
        AssignVertex(qdef, IVertex::kQdef);
        qdef->setWeight(3, 0.1);
        qdef->setWeight(2, 0.2);
        qdef->setWeight(1, 0.3);
        qdef->setWeight(0, 0.4);
        model->addVertex(qdef);
    }
    IMaterial *material = 0;
    {
        material = model->createMaterial();
        AssignMaterial(material, IMaterial::kDisableCulling |
                       IMaterial::kCastingShadow |
                       IMaterial::kCastingShadowMap |
                       IMaterial::kEnableShadowMap |
                       IMaterial::kEnableEdge |
                       IMaterial::kEnableVertexColor |
                       IMaterial::kEnableLineDraw |
                       0);
        model->addMaterial(material);
    }
    {
        material = model->createMaterial();
        AssignMaterial(material, IMaterial::kDisableCulling |
                       IMaterial::kCastingShadow |
                       IMaterial::kCastingShadowMap |
                       IMaterial::kEnableShadowMap |
                       IMaterial::kEnableEdge |
                       IMaterial::kEnableVertexColor |
                       IMaterial::kEnablePointDraw |
                       0);
        model->addMaterial(material);
    }
    IBone *bone = 0;
    {
        /* TODO: add IK constraint */
        String n1("Japanese Bone Name"), n2("English Bone Name");
        bone = model->createBone();
        bone->setName(&n1, IEncoding::kJapanese);
        bone->setName(&n2, IEncoding::kEnglish);
        bone->setOrigin(Vector3(1, 2, 3));
        bone->setDestinationOrigin(Vector3(4, 5, 6));
        bone->setFixedAxis(Vector3(0.1, 0.2, 0.3));
        bone->setFixedAxisEnable(true);
        bone->setAxisX(Vector3(0.3, 0.2, 0.1));
        bone->setAxisZ(Vector3(0.6, 0.5, 0.4));
        bone->setLocalAxesEnable(true);
        bone->setLayerIndex(42);
        bone->setExternalIndex(84);
        bone->setRotateable(true);
        bone->setMovable(true);
        bone->setVisible(true);
        bone->setInteractive(true);
        bone->setInherentOrientationEnable(true);
        bone->setInherentTranslationEnable(true);
        bone->setLocalAxesEnable(true);
        bone->setTransformAfterPhysicsEnable(true);
        bone->setTransformedByExternalParentEnable(true);
        model->addBone(bone);
    }
    {
        IMorph *morph = model->createMorph();
        AssignMorph(morph, IMorph::kVertexMorph);
        IMorph::Vertex *vmorph = new IMorph::Vertex();
        vmorph->vertex = vertex;
        vmorph->position.setValue(0.2, 0.4, 0.6);
        vmorph->index = 42;
        morph->addVertexMorph(vmorph);
        model->addMorph(morph);
    }
    if (model->type() == IModel::kPMXModel) {
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kGroupMorph);
            IMorph::Group *gmorph = new IMorph::Group();
            gmorph->morph = morph;
            gmorph->fixedWeight = 0.42;
            gmorph->index = 42;
            morph->addGroupMorph(gmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kTexCoordMorph);
            IMorph::Vertex *vmorph = new IMorph::Vertex();
            vmorph->vertex = vertex;
            vmorph->position.setValue(0.2, 0.25, 0.3);
            vmorph->index = 42;
            morph->addVertexMorph(vmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kUVA1Morph);
            IMorph::UV *vmorph = new IMorph::UV();
            vmorph->vertex = vertex;
            vmorph->position.setValue(2.0, 2.2, 2.4, 2.6);
            vmorph->index = 42;
            morph->addUVMorph(vmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kUVA2Morph);
            IMorph::UV *vmorph = new IMorph::UV();
            vmorph->vertex = vertex;
            vmorph->position.setValue(2.8, 3.0, 3.2, 3.4);
            vmorph->index = 42;
            morph->addUVMorph(vmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kUVA3Morph);
            IMorph::UV *vmorph = new IMorph::UV();
            vmorph->vertex = vertex;
            vmorph->position.setValue(3.6, 3.8, 4.0, 4.2);
            vmorph->index = 42;
            morph->addUVMorph(vmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kUVA4Morph);
            IMorph::UV *vmorph = new IMorph::UV();
            vmorph->vertex = vertex;
            vmorph->position.setValue(4.4, 4.6, 4.8, 5.0);
            vmorph->index = 42;
            morph->addUVMorph(vmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kBoneMorph);
            IMorph::Bone *bmorph = new IMorph::Bone();
            bmorph->bone = bone;
            bmorph->index = 42;
            bmorph->position.setValue(0.1, 0.2, 0.3);
            bmorph->rotation.setValue(0.4, 0.5, 0.6, 0.7);
            morph->addBoneMorph(bmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kMaterialMorph);
            IMorph::Material *mmorph = new IMorph::Material();
            mmorph->materials = new Array<IMaterial *>();
            mmorph->materials->append(material);
            mmorph->ambient.setValue(0.1, 0.2, 0.3);
            mmorph->diffuse.setValue(0.4, 0.5, 0.6, 0.7);
            mmorph->edgeColor.setValue(0.7, 0.8, 0.9, 1.0);
            mmorph->edgeSize = 0.42;
            mmorph->operation = 0;
            mmorph->shininess = 0.84;
            mmorph->specular.setValue(0.3, 0.2, 0.1);
            mmorph->sphereTextureWeight.setValue(0.4, 0.3, 0.2, 0.1);
            mmorph->textureWeight.setValue(0.5, 0.4, 0.3, 0.2);
            mmorph->toonTextureWeight.setValue(0.6, 0.5, 0.4, 0.3);
            mmorph->index = 42;
            morph->addMaterialMorph(mmorph);
            model->addMorph(morph);
        }
        {
            IMorph *morph = model->createMorph();
            AssignMorph(morph, IMorph::kFlipMorph);
            IMorph::Flip *fmorph = new IMorph::Flip();
            fmorph->morph = morph;
            fmorph->fixedWeight = 0.42;
            fmorph->index = 42;
            morph->addFlipMorph(fmorph);
            model->addMorph(morph);
        }
    }
    {
        ILabel *label = model->createLabel();
        AssignLabel(label);
        Array<IBone *> bones;
        model->getBoneRefs(bones);
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            label->addBoneRef(bone);
        }
        model->addLabel(label);
    }
    {
        ILabel *label = model->createLabel();
        AssignLabel(label);
        Array<IMorph *> morphs;
        model->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morph = morphs[i];
            label->addMorphRef(morph);
        }
        model->addLabel(label);
    }
    {
        for (int i = 0; i < int(IRigidBody::kMaxShapeType); i++) {
            for (int j = 0; j < int(IRigidBody::kMaxObjectType); j++) {
                int index = i * 3 + j;
                IRigidBody *body = model->createRigidBody();
                String n1("Japanese RigidBody Name"), n2("English RigidBody Name");
                body->setName(&n1, IEncoding::kJapanese);
                body->setName(&n2, IEncoding::kEnglish);
                body->setAngularDamping(0.1);
                body->setCollisionGroupID(index);
                body->setCollisionMask(0x1 << index);
                body->setFriction(0.3);
                body->setLinearDamping(0.4);
                body->setMass(0.5);
                body->setPosition(Vector3(0.6, 0.7, 0.8));
                body->setRestitution(0.9);
                body->setRotation(Vector3(btRadians(15), btRadians(30), btRadians(45)));
                body->setShapeType(static_cast<IRigidBody::ShapeType>(i));
                body->setSize(Vector3(1.0, 1.1, 1.2));
                body->setObjectType(static_cast<IRigidBody::ObjectType>(j));
                model->addRigidBody(body);
            }
        }
    }
    {
        IJoint *spring = model->createJoint();
        AssignJoint(spring, IJoint::kGeneric6DofSpringConstraint);
        model->addJoint(spring);
        if (model->type() == IModel::kPMXModel) {
            IJoint *sdof = model->createJoint();
            AssignJoint(sdof, IJoint::kGeneric6DofConstraint);
            model->addJoint(sdof);
            IJoint *p2p = model->createJoint();
            AssignJoint(p2p, IJoint::kPoint2PointConstraint);
            model->addJoint(p2p);
            IJoint *cone = model->createJoint();
            AssignJoint(cone, IJoint::kConeTwistConstraint);
            model->addJoint(cone);
            IJoint *slider = model->createJoint();
            AssignJoint(slider, IJoint::kSliderConstraint);
            model->addJoint(slider);
            IJoint *hinge = model->createJoint();
            AssignJoint(hinge, IJoint::kHingeConstraint);
            model->addJoint(hinge);
        }
    }
    {
        String n1("Japanese Model Name"), n2("English Model Name");
        String c1("Japanese Model Comment"), c2("English Model Comment");
        model->setVersion(2.1);
        model->setMaxUVCount(4);
        model->setName(&n1, IEncoding::kJapanese);
        model->setName(&n2, IEncoding::kEnglish);
        model->setComment(&c1, IEncoding::kJapanese);
        model->setComment(&c2, IEncoding::kEnglish);
    }
    {
        std::vector<uint8> buffer;
        buffer.resize(model->estimateSize());
        vsize written = 0;
        if (FILE *fp = fopen(filename, "wb")) {
            model->save(buffer.data(), written);
            fprintf(stderr, "%d:%d\n", int(model->estimateSize()), int(written));
            fwrite(buffer.data(), written, 1, fp);
            fclose(fp);
        }
    }
}

void CreateModelFromCSV(IModel *model, const std::string &input, const std::string &output)
{
    QFile csv(QString::fromStdString(input));
    if (csv.open(QFile::ReadOnly)) {
        QHash<QString, ILabel *> labelRefs;
        QHash<QString, IMaterial *> materialRefs;
        QHash<QString, IRigidBody *> bodyRefs;
        QScopedPointer<IString> s;
        QScopedPointer<IVertex> vertex;
        QScopedPointer<IMaterial> material;
        QScopedPointer<IBone> bone;
        QScopedPointer<IMorph> morph;
        QScopedPointer<ILabel> label;
        QScopedPointer<IRigidBody> body;
        QScopedPointer<IJoint> joint;
        QList< QPair<IVertex *, QStringList> > vertex2bones;
        Array<int> indices;
        QTextStream stream(&csv);
        stream.setCodec("Shift_JIS");
        while (!stream.atEnd()) {
            const QString &line = stream.readLine().trimmed();
            if (!line.startsWith(";")) {
                const QStringList &items = line.split(",");
                const int nitems = items.size();
                if (nitems > 0) {
                    const QString &type = items.first();
                    int index = 1;
                    if (type == "Header" && nitems == 4) {
                        model->setVersion(items.at(index++).toFloat());
                        model->setEncodingType(static_cast<IString::Codec>(items.at(index++).toInt()));
                        model->setMaxUVCount(items.at(index++).toInt());
                        Q_ASSERT(index == 4);
                    }
                    else if (type == "ModelInfo" && nitems == 5) {
                        s.reset(String::create(items.at(index++).toStdString()));
                        model->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        model->setName(s.data(), IEncoding::kEnglish);
                        s.reset(String::create(items.at(index++).toStdString()));
                        model->setComment(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        model->setComment(s.data(), IEncoding::kEnglish);
                        Q_ASSERT(index == 5);
                    }
                    else if (type == "Vertex" && nitems == 45) {
                        vertex.reset(model->createVertex());
                        items.at(index++).toInt(); // vertex index
                        vertex->setOrigin(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        vertex->setNormal(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        vertex->setEdgeSize(items.at(index++).toFloat());
                        vertex->setTextureCoord(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), 0));
                        for (int i = 1; i <= 4; i++) {
                            vertex->setOriginUV(i, Vector4(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        }
                        vertex->setType(static_cast<IVertex::Type>(items.at(index++).toInt()));
                        QPair<IVertex *, QStringList> pair;
                        pair.first = vertex.data();
                        for (int i = 0; i < 4; i++) {
                            pair.second << items.at(index++);
                            vertex2bones.append(pair);
                            vertex->setWeight(i, items.at(index++).toFloat());
                        }
                        vertex->setSdefC(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        vertex->setSdefR0(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        vertex->setSdefR1(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        model->addVertex(vertex.take());
                        Q_ASSERT(index == 45);
                    }
                    else if (type == "Material" && nitems == 31) {
                        material.reset(model->createMaterial());
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setName(s.data(), IEncoding::kEnglish);
                        material->setDiffuse(Color(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        material->setSpecular(Color(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), 1));
                        material->setShininess(items.at(index++).toFloat());
                        material->setAmbient(Color(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), 1));
                        material->setCullingDisabled(items.at(index++).toInt() != 0);
                        material->setCastingShadowEnabled(items.at(index++).toInt() != 0);
                        material->setShadowMapEnabled(items.at(index++).toInt() != 0);
                        material->setCastingShadowMapEnabled(items.at(index++).toInt() != 0);
                        material->setVertexColorEnabled(items.at(index++).toInt() != 0);
                        items.at(index++).toInt(); // primitive type
                        material->setEdgeEnabled(items.at(index++).toInt() != 0);
                        material->setEdgeSize(items.at(index++).toFloat());
                        material->setEdgeColor(Color(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setMainTexture(s.data());
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setSphereTexture(s.data());
                        material->setSphereTextureRenderMode(static_cast<IMaterial::SphereTextureRenderMode>(items.at(index++).toInt()));
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setToonTexture(s.data());
                        s.reset(String::create(items.at(index++).toStdString()));
                        material->setUserDataArea(s.data());
                        materialRefs.insert(items.at(2), material.data());
                        model->addMaterial(material.take());
                        Q_ASSERT(index == 31);
                    }
                    else if (type == "Face" && nitems == 6) {
                        index = 3;
                        indices.append(items.at(index++).toInt());
                        indices.append(items.at(index++).toInt());
                        indices.append(items.at(index++).toInt());
                        Q_ASSERT(index == 6);
                    }
                    else if (type == "Bone" && nitems == 40) {
                        bone.reset(model->createBone());
                        s.reset(String::create(items.at(index++).toStdString()));
                        bone->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        bone->setName(s.data(), IEncoding::kEnglish);
                        bone->setLayerIndex(items.at(index++).toInt());
                        bone->setTransformAfterPhysicsEnable(items.at(index++).toInt() != 0);
                        bone->setOrigin(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        bone->setMovable(items.at(index++) != 0);
                        bone->setRotateable(items.at(index++) != 0);
                        bone->setInverseKinematicsEnable(items.at(index++) != 0);
                        bone->setVisible(items.at(index++) != 0);
                        bone->setInteractive(items.at(index++) != 0);
                        s.reset(String::create(items.at(index++).toStdString()));
                        bone->setParentBoneRef(model->findBoneRef(s.data()));
                        int destinationType = items.at(index++).toInt();
                        s.reset(String::create(items.at(index++).toStdString()));
                        Vector3 destination(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                        if (destinationType != 0) {
                            bone->setDestinationOriginBoneRef(model->findBoneRef(s.data()));
                        }
                        else {
                            bone->setDestinationOrigin(destination);
                        }
                        items.at(index++).toInt(); // local inherence
                        bone->setInherentOrientationEnable(items.at(index++).toInt() != 0);
                        bone->setInherentTranslationEnable(items.at(index++).toInt() != 0);
                        bone->setInherentCoefficient(items.at(index++).toFloat());
                        s.reset(String::create(items.at(index++).toStdString()));
                        bone->setParentInherentBoneRef(model->findBoneRef(s.data()));
                        bone->setFixedAxisEnable(items.at(index++).toInt() != 0);
                        bone->setFixedAxis(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        bone->setLocalAxesEnable(items.at(index++) != 0);
                        bone->setAxisX(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        bone->setAxisZ(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        bone->setTransformedByExternalParentEnable(items.at(index++).toInt() != 0);
                        bone->setExternalIndex(items.at(index++).toInt());
                        items.at(index++); // IK target
                        items.at(index++).toInt(); // IK loop
                        items.at(index++).toFloat(); // IK degree
                        model->addBone(bone.take());
                        Q_ASSERT(index == 40);
                    }
                    else if (type == "IKLink" && nitems == 10) {
                        s.reset(String::create(items.at(index++).toStdString())); // parent bone
                        s.reset(String::create(items.at(index++).toStdString())); // link target
                        bool limitAngle = items.at(index++).toInt() != 0;
                        Q_UNUSED(limitAngle);
                        Vector3 lowerLimit, upperLimit;
                        for (int i = 0; i < 3; i++) {
                            lowerLimit[i] = qDegreesToRadians(items.at(index++).toFloat());
                            upperLimit[i] = qDegreesToRadians(items.at(index++).toFloat());
                        }
                        Q_ASSERT(index == 10);
                    }
                    else if (type == "Morph" && nitems == 5) {
                        morph.reset(model->createMorph());
                        s.reset(String::create(items.at(index++).toStdString()));
                        morph->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        morph->setName(s.data(), IEncoding::kEnglish);
                        morph->setCategory(static_cast<IMorph::Category>(items.at(index++).toInt()));
                        morph->setType(static_cast<IMorph::Type>(items.at(index++).toInt()));
                        model->addMorph(morph.take());
                        Q_ASSERT(index == 5);
                    }
                    else if (type == "VertexMorph" && nitems == 6) {
                        s.reset(String::create(items.at(index++).toStdString()));
                        if (IMorph *morph = model->findMorphRef(s.data())) {
                            QScopedPointer<IMorph::Vertex> v(new IMorph::Vertex);
                            v->index = items.at(index++).toInt();
                            v->position.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            morph->addVertexMorph(v.take());
                            Q_ASSERT(index == 6);
                        }
                    }
                    else if (type == "MaterialMorph" && nitems == 32) {
                        s.reset(String::create(items.at(index++).toStdString()));
                        if (IMorph *morph = model->findMorphRef(s.data())) {
                            QScopedPointer<IMorph::Material> v(new IMorph::Material);
                            v->materials = new Array<IMaterial *>();
                            if (IMaterial *materialRef = materialRefs.value(items.at(index++))) {
                                v->materials->append(materialRef);
                            }
                            v->operation = items.at(index++).toInt();
                            v->diffuse.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->specular.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->shininess = items.at(index++).toFloat();
                            v->ambient.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->edgeSize = items.at(index++).toFloat();
                            v->edgeColor.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->textureWeight.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->sphereTextureWeight.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            v->toonTextureWeight.setValue(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat());
                            morph->addMaterialMorph(v.take());
                            Q_ASSERT(index == 32);
                        }
                    }
                    else if (type == "Node" && nitems == 3) {
                        label.reset(model->createLabel());
                        s.reset(String::create(items.at(index++).toStdString()));
                        label->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        label->setName(s.data(), IEncoding::kEnglish);
                        labelRefs.insert(items.at(1), label.data());
                        model->addLabel(label.take());
                        Q_ASSERT(index == 3);
                    }
                    else if (type == "NodeItem" && nitems == 4) {
                        if (ILabel *labelRef = labelRefs.value(items.at(index++))) {
                            int type = items.at(index++).toInt();
                            s.reset(String::create(items.at(index++).toStdString()));
                            if (type == 0) {
                                labelRef->addBoneRef(model->findBoneRef(s.data()));
                            }
                            else if (type == 1) {
                                labelRef->addMorphRef(model->findMorphRef(s.data()));
                            }
                            Q_ASSERT(index == 4);
                        }
                    }
                    else if (type == "Body" && nitems == 22) {
                        body.reset(model->createRigidBody());
                        s.reset(String::create(items.at(index++).toStdString()));
                        body->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        body->setName(s.data(), IEncoding::kEnglish);
                        s.reset(String::create(items.at(index++).toStdString()));
                        body->setBoneRef(model->findBoneRef(s.data()));
                        body->setObjectType(static_cast<IRigidBody::ObjectType>(items.at(index++).toInt()));
                        body->setCollisionGroupID(items.at(index++).toInt());
                        const QStringList bitflags = items.at(index++).split(QRegExp("\\s+"), QString::SkipEmptyParts);
                        int flags = 0;
                        foreach (const QString &item, bitflags) {
                            int value = item.toInt();
                            if (value >= 0 && value < 16) {
                                flags |= (1 << value);
                            }
                        }
                        body->setCollisionMask(flags);
                        body->setShapeType(static_cast<IRigidBody::ShapeType>(items.at(index++).toInt()));
                        body->setSize(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        body->setPosition(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        body->setRotation(Vector3(qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat())));
                        body->setMass(items.at(index++).toFloat());
                        body->setLinearDamping(items.at(index++).toFloat());
                        body->setAngularDamping(items.at(index++).toFloat());
                        body->setRestitution(items.at(index++).toFloat());
                        body->setFriction(items.at(index++).toFloat());
                        bodyRefs.insert(items.at(1), body.data());
                        model->addRigidBody(body.take());
                        Q_ASSERT(index == 22);
                    }
                    else if (type == "Joint" && nitems == 30) {
                        joint.reset(model->createJoint());
                        s.reset(String::create(items.at(index++).toStdString()));
                        joint->setName(s.data(), IEncoding::kJapanese);
                        s.reset(String::create(items.at(index++).toStdString()));
                        joint->setName(s.data(), IEncoding::kEnglish);
                        joint->setRigidBody1Ref(bodyRefs.value(items.at(index++)));
                        joint->setRigidBody2Ref(bodyRefs.value(items.at(index++)));
                        joint->setType(static_cast<IJoint::Type>(items.at(index++).toInt()));
                        joint->setPosition(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        joint->setRotation(Vector3(qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat())));
                        joint->setPositionLowerLimit(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        joint->setPositionUpperLimit(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        joint->setRotationLowerLimit(Vector3(qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat())));
                        joint->setRotationUpperLimit(Vector3(qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat())));
                        joint->setPositionStiffness(Vector3(items.at(index++).toFloat(), items.at(index++).toFloat(), items.at(index++).toFloat()));
                        joint->setRotationStiffness(Vector3(qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat()), qDegreesToRadians(items.at(index++).toFloat())));
                        model->addJoint(joint.take());
                        Q_ASSERT(index == 30);
                    }
                    else if (type == "SoftBody" && nitems == 40) {
                    }
                }
            }
            else {
                qDebug() << line << line.split(",").size();
            }
        }
        std::vector<uint8> buffer;
        int size = int(model->estimateSize());
        buffer.resize(size);
        vsize written = 0;
        QFile pmd(QString::fromStdString(output));
        if (pmd.open(QFile::WriteOnly)) {
            model->save(buffer.data(), written);
            fprintf(stderr, "%d:%d\n", size, int(written));
            pmd.write(reinterpret_cast<const char *>(buffer.data()), written);
            pmd.close();
        }
    }
}

void AddPhoneticSymbolKeyframe(const IKeyframe::TimeIndex &startTimeIndex,
                               const IKeyframe::TimeIndex &endTimeIndex,
                               const std::string &lyric,
                               IMorph::WeightPrecision weight,
                               IMotion *motion)
{
    std::auto_ptr<IMorphKeyframe> keyframe;
    if (endTimeIndex > 0 && !lyric.empty()) {
        IKeyframe::TimeIndex offset = 3;
        String s(UnicodeString::fromUTF8(lyric));
        keyframe.reset(motion->createMorphKeyframe());
        keyframe->setName(&s);
        keyframe->setTimeIndex(startTimeIndex - offset);
        keyframe->setWeight(0.0);
        motion->addKeyframe(keyframe.release());
        keyframe.reset(motion->createMorphKeyframe());
        keyframe->setName(&s);
        keyframe->setTimeIndex(startTimeIndex);
        keyframe->setWeight(weight);
        motion->addKeyframe(keyframe.release());
        const IKeyframe::TimeIndex &peak = std::max(endTimeIndex - offset, IKeyframe::TimeIndex(startTimeIndex));
        if (peak > startTimeIndex) {
            keyframe.reset(motion->createMorphKeyframe());
            keyframe->setName(&s);
            keyframe->setTimeIndex(peak);
            keyframe->setWeight(weight);
            motion->addKeyframe(keyframe.release());
            keyframe.reset(motion->createMorphKeyframe());
            keyframe->setName(&s);
            keyframe->setTimeIndex(endTimeIndex);
            keyframe->setWeight(0.0);
            motion->addKeyframe(keyframe.release());
            qDebug()  << "keyframe:"
                      << (startTimeIndex - offset)
                      << startTimeIndex
                      << peak
                      << endTimeIndex;
        }
        else {
            keyframe.reset(motion->createMorphKeyframe());
            keyframe->setName(&s);
            keyframe->setTimeIndex(endTimeIndex + offset);
            keyframe->setWeight(0.0);
            motion->addKeyframe(keyframe.release());
            qDebug()  << "keyframe0:"
                      << (startTimeIndex - offset)
                      << startTimeIndex
                      << endTimeIndex
                      << (endTimeIndex + offset);
        }
    }
}

static const char kPhoneticSymbolA[] = { 0xe3, 0x81, 0x82, 0x0 };
static const char kPhoneticSymbolE[] = { 0xe3, 0x81, 0x88, 0x0 };
static const char kPhoneticSymbolI[] = { 0xe3, 0x81, 0x84, 0x0 };
static const char kPhoneticSymbolO[] = { 0xe3, 0x81, 0x8a, 0x0 };
static const char kPhoneticSymbolU[] = { 0xe3, 0x81, 0x86, 0x0 };
static const char kPhoneticSymbolN[] = { 0xe3, 0x82, 0x93, 0x0 };

double CreateMotionFromVSQ(IMotion *motion, const std::string &input, const std::string &output)
{
    Sequence sequence;
    FileInputStream stream(input);
    VSQFileReader reader;
    try {
        reader.read(sequence, &stream, "Shift_JIS");
    } catch (const std::exception &e) {
        VPVL2_LOG(WARNING, "exception: " << e.what());
        return 0;
    }

    const Track *masterTrack = sequence.track(0);
    const TempoList &tempo = sequence.tempoList;
    const int preferredFPS = Scene::defaultFPS();
    std::set<IKeyframe::TimeIndex> zeroDynamics;
    if (const BPList *list = masterTrack->curve("DYN")) {
        const int nlists = list->size();
        for (int i = 0; i < nlists; i++) {
            int value = list->getValue(i);
            if (value == 0) {
                const tick_t clock(list->getKeyClock(i) - sequence.getPreMeasureClocks());
                const IKeyframe::TimeIndex frameIndex(tempo.getSecFromClock(clock) * preferredFPS);
                zeroDynamics.insert(frameIndex);
            }
        }
    }
    if (const BPList *list = masterTrack->curve("OPE")) {
        const int nlists = list->size();
        for (int i = 0; i < nlists; i++) {
            tick_t clock = list->getKeyClock(i) - sequence.getPreMeasureClocks();
            int value = list->getValue(i);
            int frameIndex = int(tempo.getSecFromClock(clock) * preferredFPS);
            VPVL2_VLOG(1, "clock=" << clock << " frameIndex=" << frameIndex << " value=" << value);
        }
    }

    const Event::List *events = masterTrack->events();
    const int nevents = events->size();
    std::string previous;
    for (int i = 0; i < nevents; i++) {
        const Event *event = events->get(i);
        const Handle &handle = event->lyricHandle;
        const tick_t tick = event->clock - sequence.getPreMeasureClocks();
        const double seconds = tempo.getSecFromClock(tick);
        const IKeyframe::TimeIndex startTimeIndex = IKeyframe::TimeIndex(seconds * preferredFPS),
                noteLength = std::max(IKeyframe::TimeIndex(tempo.getSecFromClock(event->getLength()) * preferredFPS), IKeyframe::TimeIndex(1));
        if (handle.getHandleType() == HandleType::LYRIC) {
            const int nlyrics = handle.getLyricCount();
            for (int j = 0; j < nlyrics; j++) {
                const Lyric &lyric = handle.getLyricAt(j);
                const std::string &symbol = lyric.getPhoneticSymbol();
                const IMorph::WeightPrecision &dynamics = 1.0;
                IKeyframe::TimeIndex endTimeIndex = startTimeIndex + noteLength;
                if (std::lower_bound(zeroDynamics.begin(), zeroDynamics.end(), startTimeIndex) != std::lower_bound(zeroDynamics.begin(), zeroDynamics.end(), endTimeIndex)) {
                    endTimeIndex = std::min(endTimeIndex, *std::lower_bound(zeroDynamics.begin(), zeroDynamics.end(), startTimeIndex));
                }
                if (symbol.find_first_of("a") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, kPhoneticSymbolA, dynamics, motion);
                    previous.assign(kPhoneticSymbolA);
                }
                else if (symbol.find_first_of("e") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, kPhoneticSymbolE, dynamics, motion);
                    previous.assign(kPhoneticSymbolE);
                }
                else if (symbol.find_first_of("i") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, kPhoneticSymbolI, dynamics, motion);
                    previous.assign(kPhoneticSymbolI);
                }
                else if (symbol.find_first_of("o") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, kPhoneticSymbolO, dynamics, motion);
                    previous.assign(kPhoneticSymbolO);
                }
                else if (symbol.find_first_of("u") != std::string::npos || symbol.find_first_of("M") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, kPhoneticSymbolU, dynamics, motion);
                    previous.assign(kPhoneticSymbolU);
                }
                else if (symbol.find_first_of("mnN") != std::string::npos) {
                    AddPhoneticSymbolKeyframe(startTimeIndex, noteLength, kPhoneticSymbolN, dynamics, motion);
                    previous.assign(kPhoneticSymbolN);
                }
                else {
                    //VPVL2_VLOG(1, "[X] frameIndex=" << timeIndex << " value=" << lyric.getPhoneticSymbol() << " phrase=" << lyric.phrase);
                    continue;
                }
                VPVL2_VLOG(1, "[O] tick=" << tick << " clock=" << event->getLength() << " dynamics=" << event->dynamics << " frameIndex=" << startTimeIndex << " length=" << noteLength << " value=" << lyric.getPhoneticSymbol() << " phrase=" << lyric.phrase);
            }
        }
    }

    std::vector<uint8> buffer;
    int size = int(motion->estimateSize());
    buffer.resize(size);
    vsize written = 0;
    if (FILE *fp = fopen(output.c_str(), "wb")) {
        motion->save(buffer.data());
        fprintf(stderr, "%d:%d:%d\n", size, int(written), motion->countKeyframes(IKeyframe::kMorphKeyframe));
        fwrite(buffer.data(), size, 1, fp);
        fclose(fp);
    }

    return tempo.getSecFromClock(events->get(events->size() - 1)->clock);
}

double toSecondsFromTick(quint64 value, int tempo, int measure, int denominator, int resolution)
{
    return value * ((((measure / qreal(denominator)) * 60.0) / (qMax(tempo, 1) / 100.0)) / qMax(resolution, 1));
}

IKeyframe::TimeIndex toTimeIndexFromTick(quint64 value, int tempo, int measure, int denominator, int resolution)
{
    return toSecondsFromTick(value, tempo, measure, denominator, resolution) * Scene::defaultFPS();
}

double CreateMotionFromVSQX(IMotion *motion, const std::string &input, const std::string &output)
{
    QFile vsqx(QString::fromStdString(input));
    quint64 duration = 0;
    int tempo = 120, resolution = 480, measure = 4, denominator = 4;
    typedef QPair<qint64, IKeyframe::TimeIndex> TempoTimeIndex;
    typedef QMap<qint64, TempoTimeIndex> Tick2TempoTimeIndex;
    Tick2TempoTimeIndex ticks2Tempo;
    if (vsqx.open(QFile::ReadOnly)) {
        QXmlStreamReader reader(&vsqx);
        QStack<QString> stack;
        QList<IKeyframe::TimeIndex> zeroDynamics;
        typedef QPair<QString, const char *> PhoneticSymbol;
        QList<PhoneticSymbol> symbols;
        IKeyframe::TimeIndex startTimeIndex = 0, noteLength = 0;
        IMorph::WeightPrecision opening = 1.0;
        qint64 trackPosition = 0, controlPosition = 0, premeasureDuration = 0;
        const char *phoneticSymbol = 0;
        int trackIndex = 0, premeasure = 0;
        bool openingAttribute = false, dynamicsAttribute = false;
        symbols.append(PhoneticSymbol("a", kPhoneticSymbolA));
        symbols.append(PhoneticSymbol("e", kPhoneticSymbolE));
        symbols.append(PhoneticSymbol("i", kPhoneticSymbolI));
        symbols.append(PhoneticSymbol("o", kPhoneticSymbolO));
        symbols.append(PhoneticSymbol("uM", kPhoneticSymbolU));
        symbols.append(PhoneticSymbol("mNn", kPhoneticSymbolN));
        while (!reader.atEnd()) {
            QXmlStreamReader::TokenType type = reader.readNext();
            if (type == QXmlStreamReader::StartElement) {
                if (trackIndex == 0 && reader.name() == "attr") {
                    const QStringRef &id = reader.attributes().value("id");
                    openingAttribute = id == "opening";
                    dynamicsAttribute = id == "DYN";
                }
                stack.push(reader.name().toString());
            }
            else if (type == QXmlStreamReader::EndElement) {
                if (reader.name() == "tempo") {
                    QMap<qint64, TempoTimeIndex>::ConstIterator it = ticks2Tempo.lowerBound(startTimeIndex);
                    qint64 lastTicks = it != ticks2Tempo.end() ? it.key() : 0;
                    const IKeyframe::TimeIndex &lastTimeIndex = it != ticks2Tempo.end() ? it.value().second : 0;
                    const IKeyframe::TimeIndex &newTimeIndex = toTimeIndexFromTick(startTimeIndex - lastTicks, tempo, measure, denominator, resolution) + lastTimeIndex;
                    qDebug() << stack.size() << startTimeIndex << tempo << newTimeIndex;
                    ticks2Tempo.insert(startTimeIndex, TempoTimeIndex(tempo, newTimeIndex));
                }
                else if (phoneticSymbol && reader.name() == "note") {
                    IKeyframe::TimeIndex endTimeIndex = startTimeIndex + noteLength;
                    if (std::lower_bound(zeroDynamics.cbegin(), zeroDynamics.cend(), startTimeIndex) != std::lower_bound(zeroDynamics.cbegin(), zeroDynamics.cend(), endTimeIndex)) {
                        endTimeIndex = std::min(endTimeIndex, *std::lower_bound(zeroDynamics.cbegin(), zeroDynamics.cend(), startTimeIndex));
                    }
                    AddPhoneticSymbolKeyframe(startTimeIndex, endTimeIndex, phoneticSymbol, opening, motion);
                }
                openingAttribute = false;
                dynamicsAttribute = false;
                stack.pop();
            }
            else if (type == QXmlStreamReader::Characters) {
                if (stack.contains("masterTrack")) {
                    if (stack.top() == "resolution") {
                        resolution = reader.text().toInt();
                        qDebug() << "track.resolution:" << resolution;
                    }
                    else if (stack.top() == "preMeasure") {
                        premeasure = reader.text().toInt();
                        qDebug() << "track.premeasure:" << premeasureDuration;
                    }
                    else if (stack.contains("timeSig")) {
                        if (stack.top() == "nume") {
                            measure = qMax(reader.text().toInt(), 1);
                            premeasureDuration = premeasure * resolution * measure;
                            qDebug() << "measure:" << measure;
                        }
                        else if (stack.top() == "denomi") {
                            denominator = qMax(reader.text().toInt(), 1);
                            qDebug() << "denominator:" << denominator;
                        }
                    }
                    else if (stack.contains("tempo")) {
                        if (stack.top() == "bpm") {
                            tempo = reader.text().toInt();
                            qDebug() << "track.tempo.bpm:" << tempo;
                        }
                        else if (stack.top() == "posTick") {
                            startTimeIndex = reader.text().toLongLong();
                            qDebug() << "track.tempo.posTick:" << startTimeIndex;
                        }
                    }
                }
                else if (stack.contains("vsTrack")) {
                    if (trackIndex == 0 && stack.contains("musicalPart")) {
                        if (stack.contains("note")) {
                            if (stack.top() == "lyric") {
                                qDebug() << "note.lyric:" << reader.text();
                            }
                            else if (stack.top() == "phnms") {
                                qDebug() << "note.phnms:" << reader.text();
                                const QStringRef &symbol = reader.text();
                                phoneticSymbol = 0;
                                foreach (const PhoneticSymbol &sym, symbols) {
                                    foreach (const QChar &c, sym.first) {
                                        if (symbol.contains(c)) {
                                            phoneticSymbol = sym.second;
                                            break;
                                        }
                                    }
                                    if (phoneticSymbol) {
                                        break;
                                    }
                                }
                            }
                            else if (stack.top() == "posTick") {
                                qint64 ticks = (trackPosition - premeasureDuration) + reader.text().toLongLong();
                                Tick2TempoTimeIndex::ConstIterator it = ticks2Tempo.size() > 1 ? ticks2Tempo.lowerBound(ticks) - 1 : ticks2Tempo.constBegin();
                                const TempoTimeIndex &t = it.value();
                                startTimeIndex = toTimeIndexFromTick(ticks - it.key(), t.first, measure, denominator, resolution) + t.second;
                                qDebug() << "note.posTick:" << ticks << "=>" << startTimeIndex;
                            }
                            else if (stack.top() == "durTick") {
                                qint64 ticks = reader.text().toLongLong();
                                noteLength = toTimeIndexFromTick(ticks, tempo, measure, denominator, resolution);
                                qDebug() << "note.durTick:" << ticks << "=>" << noteLength;
                            }
                        }
                        else if (stack.contains("mCtrl")) {
                            if (stack.top() == "posTick") {
                                controlPosition = reader.text().toLongLong();
                            }
                            else if (dynamicsAttribute && reader.text().toInt() == 0) {
                                zeroDynamics.append(toTimeIndexFromTick(controlPosition, tempo, measure, denominator, resolution));
                                zeroDynamics = zeroDynamics.toSet().toList();
                                std::sort(zeroDynamics.begin(), zeroDynamics.end());
                            }
                        }
                        else if (stack.contains("singer")) {
                        }
                        else if (stack.top() == "posTick") {
                            trackPosition = reader.text().toLongLong();
                            qDebug() << "musicalPart.posTick:" << trackPosition << toTimeIndexFromTick(trackPosition, tempo, measure, denominator, resolution);
                        }
                        else if (stack.top() == "playTime") {
                            duration = (trackPosition + reader.text().toLongLong()) - premeasureDuration;
                            qDebug() << "musicalPart.playTime:" << duration;
                        }
                        else if (openingAttribute) {
                            opening = reader.text().toInt() / 127.0;
                        }
                    }
                    else if (stack.top() == "vsTrackNo") {
                        trackIndex = reader.text().toInt();
                    }
                }
            }
        }
        qDebug() << ticks2Tempo;
        if (reader.hasError()) {
            qWarning() << reader.errorString();
        }
    }

    std::vector<uint8> buffer;
    int size = int(motion->estimateSize());
    buffer.resize(size);
    vsize written = 0;
    QFile vmd(QString::fromStdString(output));
    if (vmd.open(QFile::WriteOnly)) {
        motion->save(buffer.data());
        fprintf(stderr, "%d:%d:%d\n", size, int(written), motion->countKeyframes(IKeyframe::kMorphKeyframe));
        vmd.write(reinterpret_cast<const char *>(buffer.data()), size);
        vmd.close();
    }

    double durationSeconds = 0;
    if (!ticks2Tempo.isEmpty()) {
        const TempoTimeIndex &value = ticks2Tempo.last();
        durationSeconds = toSecondsFromTick(duration, value.first, measure, denominator, resolution);
    }
    qDebug() << "duration:" << duration << "durationSeconds:" << durationSeconds;
    return durationSeconds;
}

double CreateMotionFromUST(IMotion *motion, const std::string &input, const std::string &output)
{
    typedef QPair<int, IKeyframe::TimeIndex> TempoTimeIndex;
    typedef QMap<int, TempoTimeIndex> Tick2TempoTimeIndex;
    Tick2TempoTimeIndex ticks2Tempo;
    QFile ust(QString::fromStdString(input));
    int ticks = 0;
    if (ust.open(QFile::ReadOnly)) {
        QTextStream stream(&ust);
        QRegExp sectionRegexp("\\[#(\\w+)\\]"), noteRegexp("\\d+");
        QString section;
        stream.setCodec("Shift_JIS");
        IKeyframe::TimeIndex notePosition = 0, noteLength = 0;
        int tempo = 120, preutterance = 127;
        const char *phoneticSymbol = 0;
        while (!stream.atEnd()) {
            const QString &line = stream.readLine();
            if (sectionRegexp.exactMatch(line)) {
                section = sectionRegexp.cap(1);
                if (noteRegexp.exactMatch(section) && phoneticSymbol) {
                    AddPhoneticSymbolKeyframe(notePosition, notePosition + noteLength, phoneticSymbol, 1.0, motion);
                }
            }
            else {
                const QStringList &pair = line.split(QChar('='));
                if (pair.size() == 2) {
                    if (section == "VERSION" && pair.first() == "Charset") {
                        stream.setCodec(qPrintable(pair.at(1)));
                    }
                    else if (section == "SETTING" && pair.first() == "Tempo") {
                        tempo = pair.at(1).toFloat() * 100;
                        ticks2Tempo.insert(0, TempoTimeIndex(tempo, 0));
                    }
                    else if (pair.first() == "Lyric") {
                        const QString &lyric = pair.at(1);
                        static const QString phoneticA("aあぁかがさざただなはばぱまやらわ");
                        static const QString phoneticI("iいぃきぎしじちぢにひびぴみり");
                        static const QString phoneticU("uうぅくぐすずつづぬふぶぷむゆるを");
                        static const QString phoneticE("eえぇけげせぜてでねへべぺめれ");
                        static const QString phoneticO("oおぉこごそぞとどのほぼぽもろを");
                        static const QString phoneticN("ん");
                        if (phoneticA.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolA;
                        }
                        else if (phoneticE.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolE;
                        }
                        else if (phoneticI.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolI;
                        }
                        else if (phoneticO.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolO;
                        }
                        else if (phoneticU.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolU;
                        }
                        else if (phoneticN.contains(lyric)) {
                            phoneticSymbol = kPhoneticSymbolN;
                        }
                        else {
                            phoneticSymbol = 0;
                        }
                        qDebug() << notePosition << lyric << phoneticSymbol;
                    }
                    else if (pair.first() == "Length") {
                        int length = pair.at(1).toInt();
                        ticks += length;
                        Tick2TempoTimeIndex::ConstIterator it = ticks2Tempo.size() > 1 ? ticks2Tempo.lowerBound(ticks) - 1 : ticks2Tempo.constBegin();
                        const TempoTimeIndex &t = it.value();
                        notePosition = toTimeIndexFromTick(ticks - it.key(), t.first, 4, 4, 480) + t.second;
                        noteLength = toTimeIndexFromTick(length, t.first, 4, 4, 480);
                    }
                    else if (pair.first() == "Tempo") {
                        tempo = pair.at(1).toFloat() * 100;
                        QMap<int, TempoTimeIndex>::ConstIterator it = ticks2Tempo.lowerBound(ticks);
                        int lastTicks = it != ticks2Tempo.end() ? it.key() : 0;
                        const IKeyframe::TimeIndex &lastTimeIndex = it != ticks2Tempo.end() ? it.value().second : 0;
                        const IKeyframe::TimeIndex &newTimeIndex = toTimeIndexFromTick(ticks - lastTicks, tempo, 4, 4, 480) + lastTimeIndex;
                        qDebug() << notePosition << tempo << newTimeIndex;
                        ticks2Tempo.insert(ticks, TempoTimeIndex(tempo, newTimeIndex));
                    }
                }
            }
        }
        AddPhoneticSymbolKeyframe(notePosition, noteLength, phoneticSymbol, preutterance, motion);
    }

    std::vector<uint8> buffer;
    int size = int(motion->estimateSize());
    buffer.resize(size);
    vsize written = 0;
    QFile vmd(QString::fromStdString(output));
    if (vmd.open(QFile::WriteOnly)) {
        motion->save(buffer.data());
        fprintf(stderr, "%d:%d:%d\n", size, int(written), motion->countKeyframes(IKeyframe::kMorphKeyframe));
        vmd.write(reinterpret_cast<const char *>(buffer.data()), size);
        vmd.close();
    }

    double durationSeconds = 0;
    if (!ticks2Tempo.isEmpty()) {
        const TempoTimeIndex &value = ticks2Tempo.last();
        durationSeconds = toSecondsFromTick(ticks, value.first, 4, 4, 480) + (value.second / float(Scene::defaultFPS()));
    }
    qDebug() << "durationSeconds:" << durationSeconds;
    return durationSeconds;
}

void CreateCameraMotion(IMotion *motion, double seconds, const std::string &output)
{
    std::vector<uint8> buffer;
    std::auto_ptr<ICameraKeyframe> keyframe(motion->createCameraKeyframe());
    Vector3 lookAt(0, 14, 0);
    keyframe->setFov(27);
    keyframe->setDistance(8);
    keyframe->setLookAt(lookAt);
    motion->addKeyframe(keyframe.release());
    keyframe.reset(motion->createCameraKeyframe());
    keyframe->setFov(27);
    keyframe->setDistance(10);
    keyframe->setLookAt(lookAt);
    keyframe->setTimeIndex(btMax(seconds, double(1)) * Scene::defaultFPS());
    motion->addKeyframe(keyframe.release());
    int size = int(motion->estimateSize());
    buffer.resize(size);
    vsize written = 0;
    if (FILE *fp = fopen(output.c_str(), "wb")) {
        motion->save(buffer.data());
        fprintf(stderr, "%d:%d:%d\n", size, int(written), motion->countKeyframes(IKeyframe::kCameraKeyframe));
        fwrite(buffer.data(), size, 1, fp);
        fclose(fp);
    }
}

}

int main(int /* argc */, char *argv[])
{
    BaseApplicationContext::initializeOnce(argv[0], 0, 2);
    Encoding::Dictionary dictionary;
    Encoding encoding(&dictionary);
    Factory factory(&encoding);
    {
        std::auto_ptr<IModel> pmd(factory.newModel(IModel::kPMDModel));
        CreateModel(pmd.get(), "output.pmd");
    }
    {
        std::auto_ptr<IModel> pmx(factory.newModel(IModel::kPMXModel));
        CreateModel(pmx.get(), "output.pmx");
    }
    if (false) {
        std::auto_ptr<IModel> pmx(factory.newModel(IModel::kPMXModel));
        CreateModelFromCSV(pmx.get(), "input.csv", "output.pmx");
    }
    if (false) {
        std::auto_ptr<IMotion> vmd(factory.newMotion(IMotion::kVMDFormat, 0));
        double seconds = CreateMotionFromVSQ(vmd.get(), "input.vsq", "output_vsq_motion.vmd");
        vmd.reset(factory.newMotion(IMotion::kVMDFormat, 0));
        CreateCameraMotion(vmd.get(), seconds, "output_vsq_camera.vmd");
    }
    if (false) {
        std::auto_ptr<IMotion> vmd(factory.newMotion(IMotion::kVMDFormat, 0));
        double seconds = CreateMotionFromVSQX(vmd.get(), "input.vsqx", "output_vsqx_motion.vmd");
        vmd.reset(factory.newMotion(IMotion::kVMDFormat, 0));
        CreateCameraMotion(vmd.get(), seconds, "output_vsqx_camera.vmd");
    }
    if (true) {
        std::auto_ptr<IMotion> vmd(factory.newMotion(IMotion::kVMDFormat, 0));
        double seconds = CreateMotionFromUST(vmd.get(), "input.ust", "output_ust_motion.vmd");
        vmd.reset(factory.newMotion(IMotion::kVMDFormat, 0));
        CreateCameraMotion(vmd.get(), seconds, "output_ust_camera.vmd");
    }
    return 0;
}
