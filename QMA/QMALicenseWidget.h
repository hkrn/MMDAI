#ifndef QMALICENSEWIDGET_H
#define QMALICENSEWIDGET_H

#include <QModelIndex>
#include <QWidget>

class QTextEdit;

class QMALicenseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMALicenseWidget(QWidget *parent = 0);
    ~QMALicenseWidget();

private slots:
    void handleDoubleClick(const QModelIndex &index);

private:
    void addLibrary(const QString &name,
                    const QString &license,
                    const QString &href,
                    const QString &path);

    QAbstractItemModel *m_model;
    QTextEdit *m_text;
    QHash<QString, QString> m_path;
};

#endif // QMALICENSEWIDGET_H
