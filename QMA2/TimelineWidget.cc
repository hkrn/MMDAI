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
                VariantInternal v;
                v.type = VariantInternal::kFace;
                v.u.face = face;
                v.name = toQString(face);
                m_keys.append(v);
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
    void registerBoneKeyFrame(const QModelIndex &index, vpvl::BoneKeyFrame *value) {
        VariantInternal v = m_keys[index.row()];
        if (v.type == VariantInternal::kBone) {
            TimelineBoneKeyFrame frame;
            frame.pack(value);
            QVariant variant;
            variant.setValue(frame);
            setData(index, variant, Qt::EditRole);
        }
        else {
            qWarning("tried registering not bone key frame: %s", v.name.toUtf8().constData());
        }
    }
    void registerFaceKeyFrame(const QModelIndex &index, vpvl::FaceKeyFrame *value) {
        VariantInternal v = m_keys[index.row()];
        if (v.type == VariantInternal::kFace) {
            TimelineFaceKeyFrame frame;
            frame.pack(value);
            QVariant variant;
            variant.setValue(frame);
            setData(index, variant, Qt::EditRole);
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

TimelineWidget::TimelineWidget(QWidget *parent) :
    QWidget(parent),
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
    setLayout(layout);
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableModel;
    delete m_tableView;
}

void TimelineWidget::registerBoneKeyFrame(vpvl::BoneKeyFrame *frame)
{
    m_tableModel->registerBoneKeyFrame(m_tableView->currentIndex(), frame);
}

void TimelineWidget::registerFaceKeyFrame(vpvl::FaceKeyFrame *frame)
{
    m_tableModel->registerFaceKeyFrame(m_tableView->currentIndex(), frame);
}

void TimelineWidget::setModel(vpvl::PMDModel *value)
{
    m_selectedModel = value;
    m_tableModel->setModel(value);
    boneDidSelect(0);
}

void TimelineWidget::selectCell(QModelIndex modelIndex)
{
    QVariant name = m_tableModel->headerData(modelIndex.row(), Qt::Vertical, Qt::DisplayRole);
    if (m_tableModel->hasModel()) {
        QByteArray bytes = internal::getTextCodec()->fromUnicode(name.toString());
        vpvl::Bone *bone = m_selectedModel->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
        if (bone)
            boneDidSelect(bone);
    }
}
