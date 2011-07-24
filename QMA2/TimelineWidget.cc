#include "TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

/* for Q_DECLARE_METATYPE restriction */
namespace internal {

struct TimelineBoneKeyFrame {
    void pack(const vpvl::BoneKeyFrame *value) {
        name = toQString(value);
        frameIndex = value->frameIndex();
        position = value->position();
        rotation = value->rotation();
    }
    void unpack(vpvl::BoneKeyFrame *value) {
        value->setName(reinterpret_cast<const uint8_t *>(name.toUtf8().constData()));
        value->setFrameIndex(frameIndex);
        value->setPosition(position);
        value->setRotation(rotation);
    }
    QString name;
    float frameIndex;
    btVector3 position;
    btQuaternion rotation;
    uint8_t interpolation[64];
};

struct TimelineFaceKeyFrame {
    void pack(const vpvl::FaceKeyFrame *value) {
        name = toQString(value);
        frameIndex = value->frameIndex();
        weight = value->weight();
    }
    void unpack(vpvl::FaceKeyFrame *value) {
        value->setName(reinterpret_cast<const uint8_t *>(name.toUtf8().constData()));
        value->setFrameIndex(frameIndex);
        value->setWeight(weight);
    }
    QString name;
    float frameIndex;
    float weight;
};

}

Q_DECLARE_METATYPE(internal::TimelineBoneKeyFrame)
Q_DECLARE_METATYPE(internal::TimelineFaceKeyFrame)

namespace internal {

class TimelineTableModel : public QAbstractTableModel {
private:
    struct VariantInternal {
        enum Type {
            kInvalid,
            kBone,
            kFace,
            kCamera
        } type;
        union {
            vpvl::Bone *bone;
            vpvl::Face *face;
        } u;
        QString name;
    };

public:
    static const QVariant kInvalidData;

    TimelineTableModel(QObject *parent = 0)
        : QAbstractTableModel(parent), m_model(0) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *value) {
        m_keys.clear();
        if (value) {
            vpvl::FaceList faces = value->faces();
            uint32_t nFaces = faces.size();
            for (uint32_t i = 0; i < nFaces; i++) {
                vpvl::Face *face = faces.at(i);
                if (face->type() != vpvl::Face::kBase) {
                    VariantInternal v;
                    v.type = VariantInternal::kFace;
                    v.u.face = face;
                    v.name = toQString(face);
                    m_keys.append(v);
                }
            }
            vpvl::BoneList bones = value->bones();
            uint32_t nBones = bones.size();
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = bones.at(i);
                if (bone->isVisible()) {
                    VariantInternal v;
                    v.type = VariantInternal::kBone;
                    v.u.bone = bone;
                    v.name = toQString(bone);
                    m_keys.append(v);
                }
            }
        }
        else {
            VariantInternal v;
            v.type = VariantInternal::kCamera;
            v.name = tr("Camera");
            m_keys.append(v);
        }
        m_model = value;
        reset();
    }

    void loadMotion(vpvl::VMDMotion *motion) {
        QModelIndex modelIndex;
        const vpvl::BoneKeyFrameList boneFrames = motion->bone().frames();
        uint32_t nBoneFrames = boneFrames.size();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::BoneKeyFrame *frame = boneFrames[i];
            const uint8_t *name = frame->name();
            vpvl::Bone *bone = m_model->findBone(name);
            uint32_t frameIndex = frame->frameIndex();
            const VariantInternal v = findVariant(bone, frameIndex, modelIndex);
            if (v.type == VariantInternal::kBone) {
                QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position());
                newFrame.setRotation(frame->rotation());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
        const vpvl::FaceKeyFrameList faceFrames = motion->face().frames();
        uint32_t nFaceFrames = faceFrames.size();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = faceFrames[i];
            const uint8_t *name = frame->name();
            vpvl::Face *bone = m_model->findFace(name);
            uint32_t frameIndex = frame->frameIndex();
            const VariantInternal v = findVariant(bone, frameIndex, modelIndex);
            if (v.type == VariantInternal::kFace) {
                QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
                vpvl::FaceKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
    }

    void loadPose(vpvl::VPDPose *pose, int frameIndex) {
        QModelIndex modelIndex;
        const vpvl::VPDPose::BoneList boneFrames = pose->bones();
        uint32_t nBoneFrames = boneFrames.size();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::VPDPose::Bone *frame = boneFrames[i];
            const uint8_t *name = frame->name;
            vpvl::Bone *bone = m_model->findBone(name);
            const VariantInternal v = findVariant(bone, frameIndex, modelIndex);
            if (v.type == VariantInternal::kBone) {
                QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
                btQuaternion rotation;
                const btVector4 &v = frame->rotation;
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position);
                newFrame.setRotation(rotation);
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
    }

    void registerBoneKeyFrame(vpvl::Bone *value, int column) {
        QModelIndex modelIndex;
        const VariantInternal v = findVariant(value, column, modelIndex);
        if (v.type == VariantInternal::kBone) {
            QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
            vpvl::BoneKeyFrame frame;
            frame.setName(value->name());
            frame.setPosition(value->position());
            frame.setRotation(value->rotation());
            frame.setFrameIndex(column);
            frame.write(reinterpret_cast<uint8_t *>(bytes.data()));
            setData(modelIndex, bytes, Qt::EditRole);
        }
        else {
            qWarning("tried registering not bone key frame: %s", v.name.toUtf8().constData());
        }
    }

    void registerFaceKeyFrame(vpvl::Face *value, int column) {
        QModelIndex modelIndex;
        const VariantInternal v = findVariant(value, column, modelIndex);
        if (v.type == VariantInternal::kFace) {
            QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
            vpvl::FaceKeyFrame frame;
            frame.setName(value->name());
            frame.setWeight(value->weight());
            frame.setFrameIndex(column);
            frame.write(reinterpret_cast<uint8_t *>(bytes.data()));
            setData(modelIndex, bytes, Qt::EditRole);
        }
        else {
            qWarning("tried registering not face key frame: %s", v.name.toUtf8().constData());
        }
    }

    bool hasModel() { return m_model != 0; }

    int rowCount(const QModelIndex & /* parent */) const {
        return m_keys.count();
    }
    int columnCount(const QModelIndex & /* parent */) const {
        return 18000;
    }
    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return kInvalidData;
        return m_values.value(QPair<int, int>(index.column(), index.row()));
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) {
        if (index.isValid() && role == Qt::EditRole) {
            m_values.insert(QPair<int, int>(index.column(), index.row()), value);
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole)
            return kInvalidData;
        if (orientation == Qt::Vertical) {
            return m_keys[section].name;
        }
        else if (orientation == Qt::Horizontal)
            return " "; //QString("%1").setNum(section + 1);
        return kInvalidData;
    }

private:
    const VariantInternal invalidVariant() {
        VariantInternal v;
        v.type = VariantInternal::kInvalid;
        return v;
    }
    const VariantInternal findVariant(const vpvl::Bone *bone, int column, QModelIndex &modelIndex) {
        if (!bone)
            return invalidVariant();
        QString s = toQString(bone);
        int i = 0;
        foreach (VariantInternal v, m_keys) {
            if (v.type == VariantInternal::kBone && s == v.name) {
                modelIndex = index(i, column);
                return v;
            }
            i++;
        }
        return invalidVariant();
    }
    const VariantInternal findVariant(const vpvl::Face *face, int column, QModelIndex &modelIndex) {
        if (!face)
            return invalidVariant();
        QString s = toQString(face);
        int i = 0;
        foreach (VariantInternal v, m_keys) {
            if (v.type == VariantInternal::kFace && s == v.name) {
                modelIndex = index(i, column);
                return v;
            }
            i++;
        }
        return invalidVariant();
    }

    vpvl::PMDModel *m_model;
    QList<VariantInternal> m_keys;
    QHash< QPair<int, int>, QVariant > m_values;
};

const QVariant TimelineTableModel::kInvalidData = QVariant();

class TimelineItemDelegate : public QStyledItemDelegate {
public:
    TimelineItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        const QRect &rect = option.rect;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        if (index.column() % 5 == 0)
            painter->fillRect(rect, Qt::lightGray);
        if (index.data() != TimelineTableModel::kInvalidData) {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(option.palette.foreground());
            QPolygon polygon;
            int width = rect.width();
            int height = width;
            int xoffset = rect.x();
            int yoffset = rect.y() + ((rect.height() - height) / 2);
            polygon.append(QPoint(xoffset, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset + height));
            polygon.append(QPoint(xoffset + width, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset ));
            painter->drawPolygon(polygon);
            painter->restore();
        }
    }
};

}

TimelineWidget::TimelineWidget(QSettings *settings, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_tableView(0),
    m_tableModel(0),
    m_selectedModel(0)
{
    m_tableModel = new internal::TimelineTableModel();
    m_tableView = new QTableView();
    m_tableView->setShowGrid(true);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    internal::TimelineItemDelegate *delegate = new internal::TimelineItemDelegate(this);
    m_tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    m_tableView->setItemDelegate(delegate);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->resizeColumnsToContents();
    connect(m_tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectCell(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tableView);
    layout->setContentsMargins(QMargins());
    setWindowTitle(tr("TimelineView"));
    setLayout(layout);
    restoreGeometry(m_settings->value("timelineWidget/geometry").toByteArray());
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableModel;
    delete m_tableView;
}

void TimelineWidget::registerBone(vpvl::Bone *bone)
{
    QModelIndexList indices = m_tableView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid())
            m_tableModel->registerBoneKeyFrame(bone, index.column());
    }
}

void TimelineWidget::registerFace(vpvl::Face *face)
{
    QModelIndexList indices = m_tableView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid())
            m_tableModel->registerFaceKeyFrame(face, index.column());
    }
}

void TimelineWidget::setModel(vpvl::PMDModel *value)
{
    m_selectedModel = value;
    m_tableModel->setModel(value);
    boneDidSelect(0);
}

void TimelineWidget::setMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    setModel(model);
    m_tableModel->loadMotion(motion);
}

void TimelineWidget::setPose(vpvl::VPDPose *pose, vpvl::PMDModel *model)
{
    QModelIndexList indices = m_tableView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid()) {
            setModel(model);
            m_tableModel->loadPose(pose, index.column());
        }
    }
}

void TimelineWidget::selectCell(QModelIndex modelIndex)
{
    QVariant name = m_tableModel->headerData(modelIndex.row(), Qt::Vertical, Qt::DisplayRole);
    if (m_tableModel->hasModel()) {
        QTextCodec *codec = internal::getTextCodec();
        QByteArray bytes = codec->fromUnicode(name.toString());
        const uint8_t *n = reinterpret_cast<const uint8_t *>(bytes.constData());
        vpvl::Bone *bone = m_selectedModel->findBone(n);
        if (bone) {
            boneDidSelect(bone);
            frameIndexSeeked(modelIndex.column());
            return;
        }
        vpvl::Face *face = m_selectedModel->findFace(n);
        if (face) {
            faceDidSelect(face);
            frameIndexSeeked(modelIndex.column());
            return;
        }
    }
}

void TimelineWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("timelineWidget/geometry", saveGeometry());
    event->accept();
}
