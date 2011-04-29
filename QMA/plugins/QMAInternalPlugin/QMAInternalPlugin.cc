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

/* under construction */

#include "QMAInternalPlugin.h"

#include <MMDAI/MMDAI.h>

const QString QMAInternalPlugin::kAllObjectNames = "MMDAI::Internal::allObjectNames";
const QString QMAInternalPlugin::kAllObjectNamesEvent = "MMDAI::Internal::allObjectNamesEvent";
const QString QMAInternalPlugin::kAllBoneNames = "MMDAI::Internal::allBoneNames";
const QString QMAInternalPlugin::kAllBoneNamesEvent = "MMDAI::Internal::allBoneNamesEvent";
const QString QMAInternalPlugin::kAllFaceNames = "MMDAI::Internal::allFaceNames";
const QString QMAInternalPlugin::kAllFaceNamesEvent = "MMDAI::Internal::allFaceNamesEvent";
const QString QMAInternalPlugin::kBoneTransform = "MMDAI::Internal::boneTransform";
const QString QMAInternalPlugin::kFaceSetWeight = "MMDAI::Internal::faceSetWeight";

static QTextCodec *g_codec = QTextCodec::codecForName("Shift-JIS");

static MMDAI::PMDBone *FindPMDBone(MMDAI::PMDModel *model, const QString &boneName)
{
    if (model != NULL) {
        int nbones = model->countBones();
        for (int i = 0; i < nbones; i++) {
            MMDAI::PMDBone *bone = model->getBoneAt(i);
            if (bone != NULL) {
                const QString name = g_codec->toUnicode(bone->getName());
                if (name == boneName) {
                    return bone;
                }
            }
        }
    }
    return NULL;
}

static MMDAI::PMDFace *FindPMDFace(MMDAI::PMDModel *model, const QString &faceName)
{
    if (model != NULL) {
        int nfaces = model->countFaces();
        for (int i = 0; i < nfaces; i++) {
            MMDAI::PMDFace *face = model->getFaceAt(i);
            if (face != NULL) {
                const QString name = g_codec->toUnicode(face->getName());
                if (name == faceName) {
                    return face;
                }
            }
        }
    }
    return NULL;
}

class PMDTableModel : public QAbstractTableModel
{
public:
    PMDTableModel(QObject *parent, const QList<QVariant> &header)
        : QAbstractTableModel(parent)
    {
        m_header  = header;
    }

    ~PMDTableModel() {
    }

    int rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return m_header.size();
    }

    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || role == Qt::DisplayRole)
            return QVariant();
        else
            return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Vertical && role == Qt::DisplayRole)
            return m_header.at(section);
        else
            return QVariant();
    }
private:
    QList<QVariant> m_header;
};

QMAInternalPlugin::QMAInternalPlugin(QObject *parent)
    : QMAPlugin(parent),
      m_controller(0),
      m_widget(0),
      m_table(0)
{
}

QMAInternalPlugin::~QMAInternalPlugin()
{
    m_controller = 0;
}

class TransformBoneLabel : public QLabel
{
public:
    TransformBoneLabel(QTableView *table, MMDAI::PMDModel *model, int type)
        : QLabel(),
          m_bone(NULL),
          m_type(type),
          m_model(model),
          m_table(table)
    {
        setAlignment(Qt::AlignCenter);
    }

    virtual void transformBone(const QPoint &diff) = 0;

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
        if (indices.size() > 0) {
            QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
            m_bone = FindPMDBone(m_model, row.toString());
            m_prev = event->pos();
        }
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (m_bone) {
            QPoint pos = event->pos();
            QPoint diff = pos - m_prev;
            transformBone(diff);
            m_model->updateBone();
            m_prev = pos;
        }
    }

    void mouseReleaseEvent(QMouseEvent *)
    {
        m_bone = NULL;
    }

    MMDAI::PMDBone *m_bone;
    const int m_type;

private:
    MMDAI::PMDModel *m_model;
    QTableView *m_table;
    QPoint m_prev;
};

class TranslateBoneLabel : public TransformBoneLabel
{
public:
    TranslateBoneLabel(QTableView *view, MMDAI::PMDModel *model, int type)
        : TransformBoneLabel(view, model, type)
    {
        setText(tr("Translate %1").arg(QChar(type)));
    }

    void transformBone(const QPoint &diff)
    {
        btVector3 pos;
        float value = diff.y() * 0.01;
        const btTransform tr(m_bone->getCurrentRotation(), m_bone->getCurrentPosition());
        switch (m_type) {
        case 'X':
            pos = tr * btVector3(value, 0.0f, 0.0f);
            break;
        case 'Y':
            pos = tr * btVector3(0.0f, value, 0.0f);
            break;
        case 'Z':
            pos = tr * btVector3(0.0f, 0.0f, value);
            break;
        }
        m_bone->setCurrentPosition(pos);
    }
};

class RotateBoneLabel : public TransformBoneLabel
{
public:
    RotateBoneLabel(QTableView *view, MMDAI::PMDModel *model, int type)
        : TransformBoneLabel(view, model, type)
    {
        setText(tr("Rotate %1").arg(QChar(type)));
    }

    void transformBone(const QPoint &diff)
    {
        btQuaternion rot, crot = m_bone->getCurrentRotation();
        float value = diff.y() * 0.1;
        switch (m_type) {
        case 'X':
            rot.setEulerZYX(MMDAIMathRadian(value), MMDAIMathRadian(0.0f), MMDAIMathRadian(0.0f));
            rot = crot * rot;
            break;
        case 'Y':
            rot.setEulerZYX(MMDAIMathRadian(0.0f), MMDAIMathRadian(value), MMDAIMathRadian(0.0f));
            rot = crot * rot;
            break;
        case 'Z':
            rot.setEulerZYX(MMDAIMathRadian(0.0f), MMDAIMathRadian(0.0f), MMDAIMathRadian(value));
            rot = crot * rot;
            break;
        }
        m_bone->setCurrentRotation(rot);
    }
};

void QMAInternalPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(baseName);
    m_controller = controller;
}

void QMAInternalPlugin::unload()
{
    m_widget->close();
    delete m_table;
    delete m_widget;
}

void QMAInternalPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    QList<QVariant> returns;
    int argc = arguments.count();
    if (command == kAllObjectNames) {
        QStringList modelNames;
        int nmodels = m_controller->countObjects();
        for (int i = 0; i < nmodels; i++) {
            MMDAI::PMDObject *object = m_controller->getObjectAt(i);
            if (object && object->isEnable()) {
                modelNames << object->getModel()->getName();
            }
        }
        returns << modelNames;
        eventPost(kAllObjectNamesEvent, returns);
    }
    else if (command == kAllBoneNames && argc >= 1) {
        MMDAI::PMDModel *model = findModel(arguments[0].toString());
        if (model != NULL) {
            QStringList boneNames;
            int nbones = model->countBones();
            for (int i = 0; i < nbones; i++) {
                MMDAI::PMDBone *bone = model->getBoneAt(i);
                boneNames << g_codec->toUnicode(bone->getName());
            }
            returns << boneNames;
            eventPost(kAllBoneNamesEvent, returns);
        }
    }
    else if (command == kAllFaceNames && argc >= 1) {
        MMDAI::PMDModel *model = findModel(arguments[0].toString());
        if (model != NULL) {
            QStringList faceNames;
            int nfaces = model->countFaces();
            for (int i = 0; i < nfaces; i++) {
                MMDAI::PMDFace *face = model->getFaceAt(i);
                faceNames << g_codec->toUnicode(face->getName());
            }
            returns << faceNames;
            eventPost(kAllFaceNamesEvent, returns);
        }
    }
}

void QMAInternalPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */

    if (type == MMDAI::ISceneEventHandler::kModelAddEvent && arguments[0].toString() == "mei") {
        m_model = findModel("Mei");
        if (m_model != NULL) {
            QList<QVariant> names;
            int nbones = m_model->countBones();
            for (int i = 0; i < nbones; i++) {
                MMDAI::PMDBone *bone = m_model->getBoneAt(i);
                names << g_codec->toUnicode(bone->getName());
            }
            int nfaces = m_model->countFaces();
            for (int i = 0; i < nfaces; i++) {
                MMDAI::PMDFace *face = m_model->getFaceAt(i);
                names << g_codec->toUnicode(face->getName());
            }

            QWidget *widget = new QWidget;
            widget->setWindowTitle("MMDAI Motion Editor");
            widget->setMinimumSize(800, 600);
            QTableView *table = new QTableView(widget);
            table->setShowGrid(true);
            QHeaderView *horizontal = table->horizontalHeader();
            horizontal->setMinimumSectionSize(10);
            horizontal->hide();
            QHeaderView *vertical = table->verticalHeader();
            vertical->setMinimumSectionSize(10);

            QLabel *label;
            QHBoxLayout *trans = new QHBoxLayout;
            label = new TranslateBoneLabel(table, m_model, 'X');
            trans->addWidget(label);
            label = new TranslateBoneLabel(table, m_model, 'Y');
            trans->addWidget(label);
            label = new TranslateBoneLabel(table, m_model, 'Z');
            trans->addWidget(label);

            QHBoxLayout *rot = new QHBoxLayout;
            label = new RotateBoneLabel(table, m_model, 'X');
            rot->addWidget(label);
            label = new RotateBoneLabel(table, m_model, 'Y');
            rot->addWidget(label);
            label = new RotateBoneLabel(table, m_model, 'Z');
            rot->addWidget(label);

            QSlider *slider = new QSlider(Qt::Horizontal);
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(weightChanged(int)));
            slider->setRange(0, 100);

            QVBoxLayout *vbox = new QVBoxLayout;
            vbox->addWidget(table);
            vbox->addLayout(trans);
            vbox->addLayout(rot);
            vbox->addWidget(slider);

            PMDTableModel *tableModel = new PMDTableModel(table, names);
            table->setModel(tableModel);

            m_widget = widget;
            m_table = table;
            m_widget->setLayout(vbox);
            m_widget->show();
        }
    }
}

MMDAI::PMDModel *QMAInternalPlugin::findModel(const QString &modelName)
{
    int nmodels = m_controller->countObjects();
    for (int i = 0; i < nmodels; i++) {
        MMDAI::PMDObject *object = m_controller->getObjectAt(i);
        if (object && object->isEnable()) {
            MMDAI::PMDModel *model = object->getModel();
            const QString name = g_codec->toUnicode(model->getName());
            if (name == modelName) {
                return model;
            }
        }
    }
    return NULL;
}

void QMAInternalPlugin::weightChanged(int value)
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        MMDAI::PMDFace *face = FindPMDFace(m_model, row.toString());
        if (face) {
            face->setWeight(value / 100.0f);
            m_model->updateFace();
        }
    }
}

MMDAI::PMDBone *QMAInternalPlugin::findBone(const QString &modelName, const QString &boneName)
{
    return FindPMDBone(findModel(modelName), boneName);
}

MMDAI::PMDFace *QMAInternalPlugin::findFace(const QString &modelName, const QString &faceName)
{
    return FindPMDFace(findModel(modelName), faceName);
}

Q_EXPORT_PLUGIN2(qma_internal_plugin, QMAInternalPlugin)
