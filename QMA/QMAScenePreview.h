#ifndef QMASCENEPREVIEW_H
#define QMASCENEPREVIEW_H

#include "QMAScenePlayer.h"

#include "MMDME/MMDME.h"

namespace {

static QTextCodec *g_codec = QTextCodec::codecForName("Shift-JIS");

MMDAI::PMDBone *QMAFindPMDBone(MMDAI::PMDModel *model, const QString &boneName)
{
    if (model != NULL) {
        const QByteArray bytes = g_codec->fromUnicode(boneName);
        return model->getBone(bytes.constData());
    }
    return NULL;
}

MMDAI::PMDFace *QMAFindPMDFace(MMDAI::PMDModel *model, const QString &faceName)
{
    if (model != NULL) {
        const QByteArray bytes = g_codec->fromUnicode(faceName);
        return model->getFace(bytes.constData());
    }
    return NULL;
}

}

class PMDTableModel : public QAbstractTableModel
{
public:
    PMDTableModel(MMDAI::PMDModel *model, QObject *parent = 0)
        : QAbstractTableModel(parent)
    {
        QList<QVariant> bones, faces;
        int nbones = model->countBoneDisplayNames();
        for (int i = 0; i < nbones; i++) {
            int name = model->getBoneDisplayNameAt(i);
            int index = model->getBoneDisplayIndexAt(i);
            MMDAI::PMDBone *parent = model->getBoneAt(name);
            MMDAI::PMDBone *child = model->getBoneAt(index);
            if (parent && child) {
                const QString parentBoneName = g_codec->toUnicode(parent->getName());
                const QString childBoneName = g_codec->toUnicode(child->getName());
                m_bones[parentBoneName] << childBoneName;
            }
        }
        QHashIterator<QString, QList<QVariant> > iterator(m_bones);
        while (iterator.hasNext()) {
            iterator.next();
            bones << iterator.key();
            foreach (const QVariant bone, iterator.value()) {
                bones << bone.toString();
            }
        }
        int nfaces = model->countFaceDisplayNames();
        for (int i = 0; i < nfaces; i++) {
            int index = model->getFaceDisplayNameAt(i);
            MMDAI::PMDFace *face = model->getFaceAt(index);
            if (face)
                faces << g_codec->toUnicode(face->getName());
        }
        m_model = model;
        m_header << bones << faces;
    }

    ~PMDTableModel()
    {
    }

    int rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return m_header.size();
    }

    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 30;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || role == Qt::DisplayRole)
            return QVariant();
        else
            return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return true;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole) {
            if (orientation == Qt::Vertical)
                return m_header.at(section);
            else if (orientation == Qt::Horizontal && (section + 1) % 5 == 0)
                return QString("%1").arg(section + 1);
            else
                return QVariant();
        }
        else {
            return QVariant();
        }
    }
private:
    MMDAI::PMDModel *m_model;
    QHash<QString, QList<QVariant> > m_bones;
    QList<QVariant> m_header;
};

class TransformBoneFrame : public QLabel
{
    Q_OBJECT

public:
    TransformBoneFrame(QTableView *table, MMDAI::PMDModel *model, int type)
        : QLabel(0),
          m_selectedBone(NULL),
          m_type(type),
          m_model(model),
          m_table(table),
          m_format("%1")
    {
        setAlignment(Qt::AlignCenter);
        connect(m_table, SIGNAL(clicked(QModelIndex)), this, SLOT(selectIndex(QModelIndex)));
    }

protected slots:
    virtual void selectBone(MMDAI::PMDBone *bone) = 0;

protected:
    virtual void transformBone(const QPoint &diff) = 0;

    void mousePressEvent(QMouseEvent *event)
    {
        QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
        if (indices.size() > 0) {
            QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
            m_selectedBone = QMAFindPMDBone(m_model, row.toString());
            m_prev = event->pos();
        }
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (m_selectedBone) {
            QPoint pos = event->pos();
            QPoint diff = pos - m_prev;
            transformBone(diff);
            m_model->updateBone();
            m_prev = pos;
        }
    }

    void mouseReleaseEvent(QMouseEvent *)
    {
        m_selectedBone = NULL;
    }

    const QString format(float value)
    {
        return m_format.arg(value, 0, 'g', 2);
    }

    MMDAI::PMDBone *m_selectedBone;
    const int m_type;

private slots:
    void selectIndex(const QModelIndex &index)
    {
        QVariant row = m_table->model()->headerData(index.row(), Qt::Vertical);
        MMDAI::PMDBone *bone = QMAFindPMDBone(m_model, row.toString());
        if (bone)
            selectBone(bone);
    }

private:
    MMDAI::PMDModel *m_model;
    QTableView *m_table;
    QString m_format;
    QPoint m_prev;
};

class TranslateBoneFrame : public TransformBoneFrame
{
    Q_OBJECT

public:
    TranslateBoneFrame(QTableView *view, MMDAI::PMDModel *model, int type)
        : TransformBoneFrame(view, model, type)
    {
        setText(tr("Translate %1").arg(QChar(type)));
    }

protected slots:
    void selectBone(MMDAI::PMDBone *bone)
    {
        const btVector3 pos(bone->getCurrentPosition());
        emit translateX(format(pos.x()));
        emit translateY(format(pos.y()));
        emit translateZ(format(pos.z()));
    }

protected:
    void transformBone(const QPoint &diff)
    {
        btVector3 pos;
        float value = diff.y() * -0.01;
        const btTransform tr(m_selectedBone->getCurrentRotation(), m_selectedBone->getCurrentPosition());
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
        emit translateX(format(pos.x()));
        emit translateY(format(pos.y()));
        emit translateZ(format(pos.z()));
        m_selectedBone->setCurrentPosition(pos);
    }

signals:
    void translateX(const QString &);
    void translateY(const QString &);
    void translateZ(const QString &);
};

class RotateBoneFrame : public TransformBoneFrame
{
    Q_OBJECT

public:
    RotateBoneFrame(QTableView *view, MMDAI::PMDModel *model, int type)
        : TransformBoneFrame(view, model, type)
    {
        setText(tr("Rotate %1").arg(QChar(type)));
    }

protected slots:
    void selectBone(MMDAI::PMDBone *bone)
    {
        const btQuaternion rot(bone->getCurrentRotation());
        emit rotateX(format(rot.x()));
        emit rotateY(format(rot.y()));
        emit rotateZ(format(rot.z()));
    }

protected:
    void transformBone(const QPoint &diff)
    {
        btQuaternion rot, crot(m_selectedBone->getCurrentRotation());
        float value = diff.y() * 0.1;
        switch (m_type) {
        case 'X':
            rot.setEulerZYX(MMDAIMathRadian(0.0f), MMDAIMathRadian(0.0f), MMDAIMathRadian(value));
            rot = crot * rot;
            break;
        case 'Y':
            rot.setEulerZYX(MMDAIMathRadian(0.0f), MMDAIMathRadian(value), MMDAIMathRadian(0.0f));
            rot = crot * rot;
            break;
        case 'Z':
            rot.setEulerZYX(MMDAIMathRadian(value), MMDAIMathRadian(0.0f), MMDAIMathRadian(0.0f));
            rot = crot * rot;
            break;
        }
        emit rotateX(format(rot.x()));
        emit rotateY(format(rot.y()));
        emit rotateZ(format(rot.z()));
        m_selectedBone->setCurrentRotation(rot);
    }

signals:
    void rotateX(const QString &);
    void rotateY(const QString &);
    void rotateZ(const QString &);
};

class CoordinateFrame : public QLineEdit
{
public:
    CoordinateFrame(QWidget *parent = 0) : QLineEdit(parent) {
        setReadOnly(true);
        setText("0");
    }
};

class QMAScenePreview : public QMAScenePlayer
{
    Q_OBJECT

public:
    QMAScenePreview(QMAPreference *preference, QWidget *parent);
    ~QMAScenePreview();

    void initialize();
    void loadPlugins();
    void start();

protected:
    void initializeGL();
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    bool handleCommand(const QString &command, const QList<QVariant> &arguments);
    bool handleEvent(const QString &type, const QList<QVariant> &arguments);

private slots:
    void updatePreview();

private:
    void drawGrid();

    QTimer m_timer;
    GLuint m_gridListID;

    Q_DISABLE_COPY(QMAScenePreview)

    void buildUI(MMDAI::PMDModel *model);
    MMDAI::PMDModel *m_model;
    QWidget *m_widget;
    QTableView *m_table;
    TranslateBoneFrame *m_tframeX;
    TranslateBoneFrame *m_tframeY;
    TranslateBoneFrame *m_tframeZ;
    RotateBoneFrame *m_rframeX;
    RotateBoneFrame *m_rframeY;
    RotateBoneFrame *m_rframeZ;
    CoordinateFrame *m_tx;
    CoordinateFrame *m_ty;
    CoordinateFrame *m_tz;
    CoordinateFrame *m_rx;
    CoordinateFrame *m_ry;
    CoordinateFrame *m_rz;
    QSlider *m_weightSlider;

private slots:
    void resetBone();
    void selectFace(const QModelIndex &index);
    void setFaceWeight(int value);
};

#endif // QMASCENEPREVIEW_H
