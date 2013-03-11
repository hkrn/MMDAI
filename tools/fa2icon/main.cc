#include <QCoreApplication>
#include <QtGui>

class QPainterWidget : public QWidget {
private:
    void paintEvent(QPaintEvent * /* event */) {
        int targets[] = {
            61440,
            61441,
            61442
        };
        QFont font("FontAwesome");
        int imageWidth = 128, fontWidth = 112;
        font.setPixelSize(fontWidth);
        QFontMetrics metrics(font);
        QStaticText text;
        QPainter painter;
        for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
            QChar c(targets[i]);
            int w = metrics.width(c);
            QImage image(imageWidth, imageWidth, QImage::Format_ARGB32);
            painter.begin(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(Qt::black);
            painter.setFont(font);
            painter.fillRect(image.rect(), Qt::transparent);
            text.setText(c);
            //painter.drawStaticText(0, (imageWidth - w), text);
            painter.drawText((imageWidth - w) / 2, fontWidth - (imageWidth - fontWidth) / 2, c);
            painter.end();
            image.save(QDir::homePath() + QString("/%1.png").arg(QVariant(c).toInt()));
        }
        qApp->exit();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPainterWidget w;
    w.show();
    return a.exec();
}
