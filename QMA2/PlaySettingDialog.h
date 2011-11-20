#ifndef PLAYSETTINGDIALOG_H
#define PLAYSETTINGDIALOG_H

#include <QtGui/QDialog>

class MainWindow;
class QCheckBox;
class QSettings;
class QSpinBox;
class SceneWidget;

class PlaySettingDialog : public QDialog
{
    Q_OBJECT

public:
    PlaySettingDialog(MainWindow *parent, QSettings *settings, SceneWidget *scene);
    ~PlaySettingDialog();

    int fromIndex() const;
    int toIndex() const;
    bool isLoop() const;

signals:
    void settingsDidSave();

private slots:
    void saveSettings();

private:
    QSettings *m_settings;
    QSpinBox *m_fromIndexBox;
    QSpinBox *m_toIndexBox;
    QCheckBox *m_loopBox;

    Q_DISABLE_COPY(PlaySettingDialog)
};

#endif // PLAYSETTINGDIALOG_H
