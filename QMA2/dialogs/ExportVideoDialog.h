#ifndef EXPORTVIDEODIALOG_H
#define EXPORTVIDEODIALOG_H

#include <QtGui/QDialog>

class MainWindow;
class QCheckBox;
class QSettings;
class QSpinBox;
class SceneWidget;

class ExportVideoDialog : public QDialog
{
    Q_OBJECT

public:
    ExportVideoDialog(MainWindow *parent, QSettings *settings, SceneWidget *scene);
    ~ExportVideoDialog();

    int sceneWidth() const;
    int sceneHeight() const;
    int fromIndex() const;
    int toIndex() const;
    bool includesGrid() const;

signals:
    void settingsDidSave();

private  slots:
    void saveSettings();

private:
    QSpinBox *m_widthBox;
    QSpinBox *m_heightBox;
    QSpinBox *m_fromIndexBox;
    QSpinBox *m_toIndexBox;
    QCheckBox *m_includeGridBox;
    QSettings *m_settings;

    Q_DISABLE_COPY(ExportVideoDialog)
};

#endif // EXPORTVIDEODIALOG_H
