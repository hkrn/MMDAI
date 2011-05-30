#include "QMAScenePreview.h"
#include "QMAPreference.h"

#include <MMDAI/MMDAI.h>

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
        qDebug() << m_bones;
        QHashIterator<QString, QList<QVariant> > iterator(m_bones);
        bones << g_codec->toUnicode(model->getBoneAt(0)->getName());
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

TransformBoneFrame::TransformBoneFrame(QTableView *table, MMDAI::PMDObject *object, int type)
    : QLabel(0),
      m_selectedBone(NULL),
      m_type(type),
      m_object(object),
      m_table(table),
      m_format("%1")
{
    setAlignment(Qt::AlignCenter);
    connect(m_table, SIGNAL(clicked(QModelIndex)), this, SLOT(selectIndex(QModelIndex)));
}

void TransformBoneFrame::mousePressEvent(QMouseEvent *event)
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        m_selectedBone = QMAFindPMDBone(m_object->getModel(), row.toString());
        m_prev = event->pos();
    }
}

void TransformBoneFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selectedBone) {
        QPoint pos = event->pos();
        QPoint diff = pos - m_prev;
        transformBone(diff);
        m_object->updateRootBone();
        m_object->getModel()->updateBone();
        m_prev = pos;
    }
}

void TransformBoneFrame::mouseReleaseEvent(QMouseEvent *)
{
    m_selectedBone = NULL;
}

const QString TransformBoneFrame::format(float value)
{
    return m_format.arg(value, 0, 'g', 2);
}

void TransformBoneFrame::selectIndex(const QModelIndex &index)
{
    QVariant row = m_table->model()->headerData(index.row(), Qt::Vertical);
    MMDAI::PMDBone *bone = QMAFindPMDBone(m_object->getModel(), row.toString());
    if (bone)
        selectBone(bone);
}

TranslateBoneFrame::TranslateBoneFrame(QTableView *view, MMDAI::PMDObject *object, int type)
    : TransformBoneFrame(view, object, type)
{
    setText(tr("Translate %1").arg(QChar(type)));
}

void TranslateBoneFrame::selectBone(MMDAI::PMDBone *bone)
{
    const btVector3 pos(bone->getCurrentPosition());
    emit translateX(format(pos.x()));
    emit translateY(format(pos.y()));
    emit translateZ(format(pos.z()));
}

void TranslateBoneFrame::transformBone(const QPoint &diff)
{
    unsigned char type = m_selectedBone->getType();
    if (type != MMDAI::ROTATE_AND_MOVE && type != MMDAI::IK_DESTINATION)
        return;
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

RotateBoneFrame::RotateBoneFrame(QTableView *view, MMDAI::PMDObject *object, int type)
    : TransformBoneFrame(view, object, type)
{
    setText(tr("Rotate %1").arg(QChar(type)));
}

void RotateBoneFrame::selectBone(MMDAI::PMDBone *bone)
{
    const btQuaternion rot(bone->getCurrentRotation());
    emit rotateX(format(rot.x()));
    emit rotateY(format(rot.y()));
    emit rotateZ(format(rot.z()));
}

void RotateBoneFrame::transformBone(const QPoint &diff)
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

QMAScenePreview::QMAScenePreview(QMAPreference *preference, QWidget *parent) :
    QMAScenePlayer(preference, parent),
    m_gridListID(0)
{
}

QMAScenePreview::~QMAScenePreview()
{
}

void QMAScenePreview::initialize()
{
    QStringList args = qApp->arguments();
    QFile file("MMDAIUserData:/MMDAI.mdf");
    if (args.count() > 1) {
        QString filename = args.at(1);
        loadUserPreference(filename);
    }
    m_preference->load(file);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updatePreview()));
}

void QMAScenePreview::loadPlugins()
{
}

void QMAScenePreview::start()
{
    addModel("/Users/hkrn/Library/MMD/MikuMikuDance_v730/UserFile/Model/miku.pmd");
    MMDAI::PMDObject *object = m_controller->getObjectAt(0);
    object->getModel()->setGlobalAlpha(1.0);
    buildUI(object);
    m_controller->setEnablePhysicsSimulation(false);
    m_timer.start(m_interval);
}

bool QMAScenePreview::handleCommand(const QString &command, const QList<QVariant> &arguments)
{
    return QMAScenePlayer::handleCommand(command, arguments);
}

bool QMAScenePreview::handleEvent(const QString &type, const QList<QVariant> &arguments)
{
    return QMAScenePlayer::handleEvent(type, arguments);
}

void QMAScenePreview::initializeGL()
{
    const float color[] = { 1.0f, 1.0f, 1.0f };
    m_preference->setFloat3(MMDAI::kPreferenceCampusColor, color);
    m_controller->initialize(width(), height());
    m_controller->updateLight();
    m_debug->initialize();
}

void QMAScenePreview::updatePreview()
{
    m_controller->updateSkin();
    m_controller->updateDepthTextureViewParam();
    update();
}

void QMAScenePreview::paintGL()
{
    QMAScenePlayer::paintGL();
    drawGrid();
}

void QMAScenePreview::mousePressEvent(QMouseEvent *event)
{
    QMAScenePlayer::mousePressEvent(event);
}

void QMAScenePreview::drawGrid()
{
    glDisable(GL_LIGHTING);
    if (m_gridListID) {
        glCallList(m_gridListID);
    }
    else {
        m_gridListID = glGenLists(1);
        glNewList(m_gridListID, GL_COMPILE);
        glColor3f(0.5f, 0.5f, 0.5f);
        GLfloat limit = 50.0f;
        // draw black grid
        for (int x = -limit; x <= limit; x += 5) {
            glBegin(GL_LINES);
            glVertex3f(x, 0.0, -limit);
            glVertex3f(x, 0.0, x == 0 ? 0.0 : limit);
            glEnd();
        }
        for (int z = -limit; z <= limit; z += 5) {
            glBegin(GL_LINES);
            glVertex3i(-limit, 0.0f, z);
            glVertex3i(z == 0 ? 0.0f : limit, 0.0f, z);
            glEnd();
        }
        // X coordinate
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(limit, 0.0f, 0.0f);
        glEnd();
        // Y coordinate
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, limit, 0.0f);
        glEnd();
        // Z coordinate
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, limit);
        glEnd();
        glEndList();
    }
    glEnable(GL_LIGHTING);
}

void QMAScenePreview::buildUI(MMDAI::PMDObject *object)
{
    QWidget *widget = new QWidget;
    widget->setWindowTitle("MMDAI Motion Editor");
    widget->setMinimumSize(800, 600);
    QTableView *table = new QTableView(widget);
    table->setShowGrid(true);
    QHeaderView *horizontal = table->horizontalHeader();
    horizontal->setDefaultSectionSize(40);
    horizontal->setResizeMode(QHeaderView::Fixed);
    QHeaderView *vertical = table->verticalHeader();
    vertical->setResizeMode(QHeaderView::Fixed);

    QGridLayout *grid = new QGridLayout;
    m_tframeX = new TranslateBoneFrame(table, object, 'X');
    m_tx = new CoordinateFrame();
    connect(m_tframeX, SIGNAL(translateX(QString)), m_tx, SLOT(setText(QString)));
    grid->addWidget(m_tframeX, 0, 0);
    grid->addWidget(m_tx, 0, 1);
    m_tframeY = new TranslateBoneFrame(table, object, 'Y');
    m_ty = new CoordinateFrame();
    connect(m_tframeY, SIGNAL(translateY(QString)), m_ty, SLOT(setText(QString)));
    grid->addWidget(m_tframeY, 0, 2);
    grid->addWidget(m_ty, 0, 3);
    m_tframeZ = new TranslateBoneFrame(table, object, 'Z');
    m_tz = new CoordinateFrame();
    connect(m_tframeZ, SIGNAL(translateZ(QString)), m_tz, SLOT(setText(QString)));
    grid->addWidget(m_tframeZ, 0, 4);
    grid->addWidget(m_tz, 0, 5);

    m_rframeX = new RotateBoneFrame(table, object, 'X');
    m_rx = new CoordinateFrame();
    connect(m_rframeX, SIGNAL(rotateX(QString)), m_rx, SLOT(setText(QString)));
    grid->addWidget(m_rframeX, 1, 0);
    grid->addWidget(m_rx, 1, 1);
    m_rframeY = new RotateBoneFrame(table, object, 'Y');
    m_ry = new CoordinateFrame();
    connect(m_rframeY, SIGNAL(rotateY(QString)), m_ry, SLOT(setText(QString)));
    grid->addWidget(m_rframeY, 1, 2);
    grid->addWidget(m_ry, 1, 3);
    m_rframeZ = new RotateBoneFrame(table, object, 'Z');
    m_rz = new CoordinateFrame();
    connect(m_rframeZ, SIGNAL(rotateZ(QString)), m_rz, SLOT(setText(QString)));
    grid->addWidget(m_rframeZ, 1, 4);
    grid->addWidget(m_rz, 1, 5);

    QHBoxLayout *hbox = new QHBoxLayout;
    m_weightSlider = new QSlider(Qt::Horizontal);
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(selectFace(QModelIndex)));
    connect(m_weightSlider, SIGNAL(valueChanged(int)), this, SLOT(setFaceWeight(int)));
    m_weightSlider->setRange(0, 100);
    hbox->addWidget(m_weightSlider);
    QPushButton *button = new QPushButton(tr("Initialize bone"));
    connect(button, SIGNAL(clicked()), this, SLOT(resetBone()));
    hbox->addWidget(button);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(table);
    vbox->addLayout(grid);
    vbox->addLayout(hbox);

    PMDTableModel *tableModel = new PMDTableModel(object->getModel());
    table->setModel(tableModel);

    m_widget = widget;
    m_table = table;
    m_object = object;
    m_widget->setLayout(vbox);
    m_widget->show();
    show();
}

void QMAScenePreview::resetBone()
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        MMDAI::PMDBone *bone = QMAFindPMDBone(m_object->getModel(), row.toString());
        if (bone) {
            const QString text("0");
            m_tx->setText(text);
            m_ty->setText(text);
            m_tz->setText(text);
            m_rx->setText(text);
            m_ry->setText(text);
            m_rz->setText(text);
            bone->reset();
            m_object->updateRootBone();
            m_object->getModel()->updateBone();
            m_object->updateSkin();
        }
    }
}

void QMAScenePreview::selectFace(const QModelIndex &index)
{
    QVariant row = m_table->model()->headerData(index.row(), Qt::Vertical);
    MMDAI::PMDFace *face = QMAFindPMDFace(m_object->getModel(), row.toString());
    if (face)
        m_weightSlider->setValue(face->getWeight() * 100.0f);
}

void QMAScenePreview::setFaceWeight(int value)
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        MMDAI::PMDFace *face = QMAFindPMDFace(m_object->getModel(), row.toString());
        if (face) {
            face->setWeight(value / 100.0f);
            m_object->getModel()->updateFace();
        }
    }
}
