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

class TransformBoneFrame : public QLabel
{
    Q_OBJECT

public:
    TransformBoneFrame(QTableView *table, MMDAI::PMDObject *object, int type);
    virtual ~TransformBoneFrame() {}

protected slots:
    virtual void selectBone(MMDAI::PMDBone *bone) = 0;

protected:
    virtual void transformBone(const QPoint &diff) = 0;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    const QString format(float value);

    MMDAI::PMDBone *m_selectedBone;
    const int m_type;

private slots:
    void selectIndex(const QModelIndex &index);

private:
    MMDAI::PMDObject *m_object;
    QTableView *m_table;
    QString m_format;
    QPoint m_prev;
};

class TranslateBoneFrame : public TransformBoneFrame
{
    Q_OBJECT

public:
    TranslateBoneFrame(QTableView *view, MMDAI::PMDObject *object, int type);

protected slots:
    void selectBone(MMDAI::PMDBone *bone);

protected:
    void transformBone(const QPoint &diff);

signals:
    void translateX(const QString &);
    void translateY(const QString &);
    void translateZ(const QString &);
};

class RotateBoneFrame : public TransformBoneFrame
{
    Q_OBJECT

public:
    RotateBoneFrame(QTableView *view, MMDAI::PMDObject *object, int type);

protected slots:
    void selectBone(MMDAI::PMDBone *bone);

protected:
    void transformBone(const QPoint &diff);

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

    void buildUI(MMDAI::PMDObject *object);
    MMDAI::PMDObject *m_object;
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
