#ifndef QMASCENEPREVIEW_H
#define QMASCENEPREVIEW_H

#include "QMAScenePlayer.h"

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
    bool handleCommand(const QString &command, const QList<QVariant> &arguments);
    bool handleEvent(const QString &type, const QList<QVariant> &arguments);

private slots:
    void updatePreview();

private:
    void drawGrid();

    QTimer m_timer;
    GLuint m_gridListID;

    Q_DISABLE_COPY(QMAScenePreview)
};

#endif // QMASCENEPREVIEW_H
