#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/BaseApplicationContext.h> /* BaseApplicationContext::initializeOnce */
#include <vpvl2/extensions/icu4c/Encoding.h>

#include <stdio.h>

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;

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
    {
        IMaterial *material = model->createMaterial();
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
    IMaterial *material = 0;
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
        bone->setAxisFixedEnable(true);
        bone->setAxisX(Vector3(0.3, 0.2, 0.1));
        bone->setAxisZ(Vector3(0.6, 0.5, 0.4));
        bone->setLocalAxesEnable(true);
        bone->setLayerIndex(42);
        bone->setExternalIndex(84);
        bone->setRotateable(true);
        bone->setMovable(true);
        bone->setVisible(true);
        bone->setInteractive(true);
        bone->setHasInverseKinematics(true);
        bone->setInherentOrientationEnable(true);
        bone->setInherentTranslationEnable(true);
        bone->setLocalAxesEnable(true);
        bone->setTransformAfterPhysicsEnable(true);
        bone->setTransformedByExternalParentEnable(true);
        model->addBone(bone);
    }
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
        AssignMorph(morph, IMorph::kVertexMorph);
        IMorph::Vertex *vmorph = new IMorph::Vertex();
        vmorph->vertex = vertex;
        vmorph->position.setValue(0.2, 0.4, 0.6);
        vmorph->index = 42;
        morph->addVertexMorph(vmorph);
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
            fprintf(stderr, "%ld:%ld\n", model->estimateSize(), written);
            fwrite(buffer.data(), written, 1, fp);
            fclose(fp);
        }
    }
}

}

int main(int /* argc */, char *argv[])
{
    BaseApplicationContext::initializeOnce(argv[0], 0, 2);
    Encoding::Dictionary dictionary;
    Encoding encoding(&dictionary);
    Factory factory(&encoding);
    IModel *pmd = factory.newModel(IModel::kPMDModel);
    CreateModel(pmd, "output.pmd");
    delete pmd;
    IModel *pmx = factory.newModel(IModel::kPMXModel);
    CreateModel(pmx, "output.pmx");
    delete pmx;
    return 0;
}
