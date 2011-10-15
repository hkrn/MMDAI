#include "shared.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

class ExtendedUI : public UI
{
public:
    ExtendedUI()
        : m_writer(0)
    {
    }
    ~ExtendedUI() {
        delete m_writer;
    }

protected:
    virtual void keyPressEvent(QKeyEvent *event) {
        switch (event->key()) {
        case Qt::Key_R: {
            if (m_writer)
                break;
            QString path = QDir::homePath() + "/test.avi";
            cv::Size size(width(), height());
            int fourcc = CV_FOURCC('a', 'v', 'c', '1');
            m_writer = new cv::VideoWriter();
            bool ret = m_writer->open(path.toUtf8().constData(), fourcc, 29.97, size);
            qDebug() << "Started capturing:" << ret;
            break;
        }
        case Qt::Key_S: {
            delete m_writer;
            m_writer = 0;
            qDebug() << "Stopped capturing";
            break;
        }
        }
    }
    virtual void paintGL() {
        UI::paintGL();
        if (m_writer) {
            QImage image = grabFrameBuffer(true);
            cv::Mat mat(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
            cv::Mat mat2(mat.rows, mat.cols, CV_8UC3);
            int fromTo[] = { 0, 0, 1, 1, 2, 2 };
            cv::mixChannels(&mat, 1, &mat2, 1, fromTo, 3);
            m_writer->write(mat2);
        }
    }

private:
    cv::VideoWriter *m_writer;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#if 1
    ExtendedUI ui; ui.show();
    return app.exec();
#else
    ExtendedUI *ui = new UI();
    ui->show();
    delete ui;
    return 0;
#endif
}

