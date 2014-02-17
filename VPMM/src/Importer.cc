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

#include "Importer.h"
#include "ProjectProxy.h"

#include <QtCore>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

namespace {

class Worker : public QObject, public QRunnable {
    Q_OBJECT

public:
    Worker(const Factory *factoryRef, const QUrl &url, QObject *parent)
        : QObject(parent),
          m_factoryRef(factoryRef),
          m_fileUrl(url)
    {
        Q_ASSERT(url.isValid());
    }
    ~Worker() {
    }

signals:
    void modelDidLoad(IModel *model, const QUrl &fileUrl, const QString &errorString);

private:
    void run() {
        Assimp::Importer importer;
        QScopedPointer<IModel> model(m_factoryRef->newModel(IModel::kPMXModel));
        const QByteArray path = m_fileUrl.toLocalFile().toUtf8();
        if (const aiScene *scene = importer.ReadFile(path.constData(), aiProcessPreset_TargetRealtime_Quality)) {
            QHash<int, int> material2Indices;
            Array<int> indices;
            int numIndices = 0;
            const int nmeshes = scene->mNumMeshes;
            for (int i = 0; i < nmeshes; i++) {
                const aiMesh *mesh = scene->mMeshes[i];
                convertBones(mesh, model);
                convertIndices(mesh, numIndices, indices);
                convertVertices(mesh, model);
                if (!material2Indices.contains(mesh->mMaterialIndex)) {
                    material2Indices.insert(mesh->mMaterialIndex, numIndices);
                }
            }
            model->setIndices(indices);
            IMaterial::IndexRange range;
            const int nmaterials = scene->mNumMaterials;
            for (int i = 0; i < nmaterials; i++) {
                const aiMaterial *material = scene->mMaterials[i];
                range.count = material2Indices.value(i);
                convertMaterial(material, range, model);
            }
            setModelInfo(model);
            emit modelDidLoad(model.take(), m_fileUrl, QString());
        }
        else {
            VPVL2_LOG(WARNING, importer.GetErrorString());
            emit modelDidLoad(0, QUrl(), importer.GetErrorString());
        }
    }
    void setModelInfo(QScopedPointer<IModel> &model) {
        QScopedPointer<IString> string;
        string.reset(String::create(QFileInfo(m_fileUrl.toLocalFile()).fileName().toStdString()));
        model->setEncodingType(IString::kUTF8);
        model->setName(string.data(), IEncoding::kDefaultLanguage);
        model->setName(string.data(), IEncoding::kEnglish);
        string.reset(String::create(QStringLiteral("Imported by VPMM at %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).toStdString()));
        model->setComment(string.data(), IEncoding::kDefaultLanguage);
        model->setComment(string.data(), IEncoding::kEnglish);
    }
    static void convertBones(const aiMesh *mesh, QScopedPointer<IModel> &model) {
        if (mesh->HasBones()) {
            QScopedPointer<IBone> bone;
            QScopedPointer<IString> string;
            aiVector3D scaling, translation;
            aiQuaternion orientation;
            const int nbones = mesh->mNumBones;
            for (int i = 0; i < nbones; i++) {
                const aiBone *sourceBone = mesh->mBones[i];
                sourceBone->mOffsetMatrix.Decompose(scaling, orientation, translation);
                string.reset(String::create(sourceBone->mName.C_Str()));
                bone.reset(model->createBone());
                bone->setName(string.data(), IEncoding::kDefaultLanguage);
                bone->setName(string.data(), IEncoding::kEnglish);
                bone->setOrigin(Vector3(translation.x, translation.y, translation.z));
                model->addBone(bone.take());
            }
        }
    }
    void convertIndices(const aiMesh *mesh, int &numIndices, Array<int> &indices) {
        if (mesh->HasFaces()) {
            const int nfaces = mesh->mNumFaces;
            numIndices = 0;
            for (int i = 0; i < nfaces; i++) {
                const aiFace &face = mesh->mFaces[i];
                const int nindices = face.mNumIndices;
                for (int j = 0; j < nindices; j++) {
                    int index = face.mIndices[j];
                    indices.append(index);
                }
                numIndices += nindices;
            }
        }
    }
    static void convertVertices(const aiMesh *mesh, QScopedPointer<IModel> &model) {
        if (mesh->HasPositions()) {
            const int nvertices = mesh->mNumVertices;
            QScopedPointer<IVertex> vertex;
            for (int i = 0; i < nvertices; i++) {
                const aiVector3D &origin = mesh->mVertices[i];
                vertex.reset(model->createVertex());
                vertex->setOrigin(Vector3(origin.x, origin.y, origin.z));
                if (mesh->HasNormals()) {
                    const aiVector3D &normal = mesh->mNormals[i];
                    vertex->setNormal(Vector3(normal.x, normal.y, normal.z));
                }
                if (mesh->HasTangentsAndBitangents()) {
                    //const aiVector3D &tangent = mesh->mTangents[i];
                }
                if (mesh->HasTextureCoords(0)) {
                    const aiVector3D &texcoord = mesh->mTextureCoords[0][i];
                    vertex->setTextureCoord(Vector3(texcoord.x, texcoord.y, texcoord.z));
                }
                if (mesh->HasVertexColors(0)) {
                    const aiColor4D &color = mesh->mColors[0][i];
                    vertex->setOriginUV(0, Vector4(color.r, color.g, color.b, color.a));
                }
                model->addVertex(vertex.take());
            }
        }
    }
    static void convertMaterial(const aiMaterial *sourceMaterial, IMaterial::IndexRange &range, QScopedPointer<IModel> &model) {
        QScopedPointer<IMaterial> material;
        QScopedPointer<IString> string;
        aiString name, texturePath;
        aiColor4D ambient, diffuse, specular, emissive;
        float shininess;
        int twoside;
        material.reset(model->createMaterial());
        sourceMaterial->Get(AI_MATKEY_NAME, name);
        string.reset(String::create(name.C_Str()));
        material->setName(string.data(), IEncoding::kDefaultLanguage);
        material->setName(string.data(), IEncoding::kEnglish);
        sourceMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        sourceMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
        ambient += emissive;
        material->setAmbient(Color(ambient.r, ambient.g, ambient.b, ambient.a));
        sourceMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->setDiffuse(Color(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
        sourceMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->setSpecular(Color(specular.r, specular.g, specular.b, specular.a));
        sourceMaterial->Get(AI_MATKEY_SHININESS, shininess);
        material->setShininess(shininess);
        sourceMaterial->Get(AI_MATKEY_TWOSIDED, twoside);
        material->setCullingDisabled(twoside != 0);
        sourceMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
        string.reset(String::create(texturePath.C_Str()));
        material->setMainTexture(string.data());
        range.end = range.start + range.count;
        material->setIndexRange(range);
        range.start += range.count;
        model->addMaterial(material.take());
    }

    const Factory *m_factoryRef;
    QUrl m_fileUrl;
};

}

Importer::Importer()
{
    Assimp::Importer importer;
    std::string extensions;
    importer.GetExtensionList(extensions);
    foreach (const QString &extension, QString::fromStdString(extensions).split(";")) {
        m_nameFilters.append(QStringLiteral("%1 (%1)").arg(extension));
    }
}

Importer::~Importer()
{
}

void Importer::importModel(const QUrl &url)
{
    Worker *worker = new Worker(m_projectRef->factoryInstanceRef(), url, this);
    connect(worker, &Worker::modelDidLoad, this, &Importer::internalLoadModel);
    QThreadPool::globalInstance()->start(worker);
}

QStringList Importer::nameFilters() const
{
    return m_nameFilters;
}

void Importer::internalLoadModel(IModel *model, const QUrl &fileUrl, const QString errorString)
{
    if (model) {
        QByteArray bytes;
        vsize written;
        bytes.reserve(model->estimateSize());
        model->save(reinterpret_cast<uint8 *>(bytes.data()), written);
        QDir dir(fileUrl.toLocalFile());
        dir.cdUp();
        QFile file(dir.absoluteFilePath("test.pmx"));
        qDebug() << file.fileName();
        file.open(QFile::WriteOnly);
        file.write(bytes.constData(), written);
        file.close();
        delete model;
    }
    else {
        VPVL2_LOG(WARNING, "Failed importing the model: " << errorString.toStdString());
    }
}

#include "Importer.moc"
