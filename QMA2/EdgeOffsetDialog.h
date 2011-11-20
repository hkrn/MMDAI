#ifndef EDGEOFFSETDIALOG_H
#define EDGEOFFSETDIALOG_H

#include <QtGui/QDialog>

namespace vpvl {
class PMDModel;
}

class QDoubleSpinBox;
class MainWindow;
class SceneWidget;

class EdgeOffsetDialog : public QDialog
{
    Q_OBJECT

public:
    EdgeOffsetDialog(MainWindow *parent, SceneWidget *scene);
    ~EdgeOffsetDialog();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setEdgeOffset(double value);
    void commit();
    void rollback();

private:
    QDoubleSpinBox *m_spinBox;
    SceneWidget *m_sceneWidget;
    vpvl::PMDModel *m_selected;
    float m_edgeOffset;

    Q_DISABLE_COPY(EdgeOffsetDialog)
};

#endif // EDGEOFFSETDIALOG_H
