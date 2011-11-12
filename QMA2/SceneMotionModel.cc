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

#include "SceneMotionModel.h"
#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace {

class RootTreeItem : public MotionBaseModel::ITreeItem
{
public:
    RootTreeItem()
    {
    }
    ~RootTreeItem() {
        qDeleteAll(m_children);
    }

    void addChild(ITreeItem *item) {
        m_children.append(item);
    }
    ITreeItem *parent() const {
        return 0;
    }
    ITreeItem *child(int row) const {
        return m_children.value(row);
    }
    const QString &name() const {
        static const QString empty = "";
        return empty;
    }
    bool isRoot() const {
        return true;
    }
    bool isCategory() const {
        return false;
    }
    int rowIndex() const {
        return 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<ITreeItem *> m_children;
};

class CameraTreeItem : public MotionBaseModel::ITreeItem
{
public:
    CameraTreeItem(RootTreeItem *root)
        : m_parent(root)
    {
    }
    ~CameraTreeItem() {
    }

    void addChild(ITreeItem * /* item */) {
    }
    ITreeItem *parent() const {
        return m_parent;
    }
    ITreeItem *child(int /* row */) const {
        return 0;
    }
    const QString &name() const {
        static const QString name = "Camera";
        return name;
    }
    bool isRoot() const {
        return false;
    }
    bool isCategory() const {
        return true;
    }
    int rowIndex() const {
        return 0;
    }
    int countChildren() const {
        return 0;
    }

private:
    ITreeItem *m_parent;
};

}

SceneMotionModel::SceneMotionModel(QUndoGroup *undo, QObject *parent)
    : MotionBaseModel(undo, parent),
      m_root(0),
      m_camera(0)
{
    RootTreeItem *root = new RootTreeItem();
    CameraTreeItem *camera = new CameraTreeItem(root);
    root->addChild(camera);
    m_root = root;
    m_camera = camera;
}

SceneMotionModel::~SceneMotionModel()
{
    delete m_camera;
    delete m_root;
}

QVariant SceneMotionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole && index.column() == 0) {
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        return item->name();
    }
    else if (role == kBinaryDataRole) {
        QVariant value;
        if (index.row() == m_camera->rowIndex())
            value = m_cameraData.value(index);
        return value;
    }
    else {
        return QVariant();
    }
}

bool SceneMotionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (index.row() == m_camera->rowIndex()) {
            m_cameraData.insert(index, value);
            setModified(true);
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

int SceneMotionModel::columnCount(const QModelIndex & /* parent */) const
{
    return maxFrameCount() + 2;
}

int SceneMotionModel::maxFrameCount() const
{
    return 54000;
}

const QModelIndex SceneMotionModel::frameIndexToModelIndex(ITreeItem *item, int frameIndex) const
{
    int rowIndex = item->rowIndex();
    QModelIndex modelIndex;
    if (rowIndex == 0) {
        modelIndex = index(rowIndex, toModelIndex(frameIndex), QModelIndex());
        if (!modelIndex.isValid())
            createIndex(rowIndex, frameIndex, item);
    }
    else {
        const QModelIndex &parentIndex = index(item->parent()->rowIndex(), 0);
        modelIndex = index(rowIndex, toModelIndex(frameIndex), parentIndex);
        if (!modelIndex.isValid())
            createIndex(rowIndex, frameIndex, item);
    }
    return modelIndex;
}

void SceneMotionModel::saveMotion(vpvl::VMDMotion * /* motion */)
{
}

void SceneMotionModel::copyFrames(int /* frameIndex */)
{
}

void SceneMotionModel::pasteFrame(int /* frameIndex */)
{
}

void SceneMotionModel::selectByModelIndex(const QModelIndex & /* index */)
{
}

const QByteArray SceneMotionModel::nameFromModelIndex(const QModelIndex & /* index */) const
{
    return QByteArray();
}

void SceneMotionModel::loadMotion(vpvl::VMDMotion *motion)
{
    const vpvl::CameraAnimation &animation = motion->cameraAnimation();
    const int nCameraFrames = animation.countKeyFrames();
    for (int i = 0; i < nCameraFrames; i++) {
        const vpvl::CameraKeyFrame *frame = animation.frameAt(i);
        int frameIndex = static_cast<int>(frame->frameIndex());
        QByteArray bytes(vpvl::CameraKeyFrame::strideSize(), '0');
        const QModelIndex &modelIndex = frameIndexToModelIndex(m_camera, frameIndex);
        // It seems to use CameraKeyFrame#clone()
        vpvl::CameraKeyFrame newFrame;
        newFrame.setPosition(frame->position());
        newFrame.setAngle(frame->angle());
        newFrame.setFovy(frame->fovy());
        newFrame.setDistance(frame->distance());
        newFrame.setFrameIndex(frameIndex);
        vpvl::QuadWord v;
        for (int i = 0; i < vpvl::CameraKeyFrame::kMax; i++) {
            vpvl::CameraKeyFrame::InterpolationType type = static_cast<vpvl::CameraKeyFrame::InterpolationType>(i);
            frame->getInterpolationParameter(type, v);
            newFrame.setInterpolationParameter(type, v);
        }
        newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
        setData(modelIndex, bytes);
    }
}

void SceneMotionModel::removeMotion()
{
}

void SceneMotionModel::deleteFrameByModelIndex(const QModelIndex & /* index */)
{
}
