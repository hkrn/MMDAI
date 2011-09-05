#ifndef LICENSEWIDGET_H
#define LICENSEWIDGET_H

#include <QModelIndex>
#include <QWidget>

class QTextEdit;

class LicenseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LicenseWidget(QWidget *parent = 0);
    ~LicenseWidget();

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

