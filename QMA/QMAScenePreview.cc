#include "QMAScenePreview.h"
#include "QMAPreference.h"

#include <MMDAI/MMDAI.h>

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
    MMDAI::PMDModel *model = object->getModel();
    model->setGlobalAlpha(1.0);
    buildUI(model);
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

void QMAScenePreview::buildUI(MMDAI::PMDModel *model)
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
    m_tframeX = new TranslateBoneFrame(table, model, 'X');
    m_tx = new CoordinateFrame();
    connect(m_tframeX, SIGNAL(translateX(QString)), m_tx, SLOT(setText(QString)));
    grid->addWidget(m_tframeX, 0, 0);
    grid->addWidget(m_tx, 0, 1);
    m_tframeY = new TranslateBoneFrame(table, model, 'Y');
    m_ty = new CoordinateFrame();
    connect(m_tframeY, SIGNAL(translateY(QString)), m_ty, SLOT(setText(QString)));
    grid->addWidget(m_tframeY, 0, 2);
    grid->addWidget(m_ty, 0, 3);
    m_tframeZ = new TranslateBoneFrame(table, model, 'Z');
    m_tz = new CoordinateFrame();
    connect(m_tframeZ, SIGNAL(translateZ(QString)), m_tz, SLOT(setText(QString)));
    grid->addWidget(m_tframeZ, 0, 4);
    grid->addWidget(m_tz, 0, 5);

    m_rframeX = new RotateBoneFrame(table, model, 'X');
    m_rx = new CoordinateFrame();
    connect(m_rframeX, SIGNAL(rotateX(QString)), m_rx, SLOT(setText(QString)));
    grid->addWidget(m_rframeX, 1, 0);
    grid->addWidget(m_rx, 1, 1);
    m_rframeY = new RotateBoneFrame(table, model, 'Y');
    m_ry = new CoordinateFrame();
    connect(m_rframeY, SIGNAL(rotateY(QString)), m_ry, SLOT(setText(QString)));
    grid->addWidget(m_rframeY, 1, 2);
    grid->addWidget(m_ry, 1, 3);
    m_rframeZ = new RotateBoneFrame(table, model, 'Z');
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

    PMDTableModel *tableModel = new PMDTableModel(model);
    table->setModel(tableModel);

    m_widget = widget;
    m_table = table;
    m_model = model;
    m_widget->setLayout(vbox);
    m_widget->show();
    show();
}

void QMAScenePreview::resetBone()
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        MMDAI::PMDBone *bone = QMAFindPMDBone(m_model, row.toString());
        if (bone) {
            const QString text("0");
            m_tx->setText(text);
            m_ty->setText(text);
            m_tz->setText(text);
            m_rx->setText(text);
            m_ry->setText(text);
            m_rz->setText(text);
            bone->reset();
            m_model->updateBone();
        }
    }
}

void QMAScenePreview::selectFace(const QModelIndex &index)
{
    QVariant row = m_table->model()->headerData(index.row(), Qt::Vertical);
    MMDAI::PMDFace *face = QMAFindPMDFace(m_model, row.toString());
    if (face)
        m_weightSlider->setValue(face->getWeight() * 100.0f);
}

void QMAScenePreview::setFaceWeight(int value)
{
    QModelIndexList indices = m_table->selectionModel()->selectedIndexes();
    if (indices.size() > 0) {
        QVariant row = m_table->model()->headerData(indices.at(0).row(), Qt::Vertical);
        MMDAI::PMDFace *face = QMAFindPMDFace(m_model, row.toString());
        if (face) {
            face->setWeight(value / 100.0f);
            m_model->updateFace();
        }
    }
}
