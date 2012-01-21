/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "VideoEncoder.h"
#include <QtCore/QtCore>

#ifdef OPENCV_FOUND
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

bool VideoEncoder::isSupported()
{
#ifdef OPENCV_FOUND
    return true;
#else
    return false;
#endif
}

VideoEncoder::VideoEncoder(const QString &filename,
                           const QSize &size,
                           double fps,
                           QObject *parent)
    : QThread(parent),
      m_writer(0),
      m_size(size),
      m_running(true)
{
#ifdef Q_OS_MACX
        /* MacOSX では非圧縮の場合どうも QTKit では DIB ではなく PNG で書き出さないとうまくいかない様子 */
        int fourcc = CV_FOURCC('p', 'n', 'g', ' ');
#else
        int fourcc = CV_FOURCC('D', 'I', 'B', ' ');
#endif
    m_writer = new cv::VideoWriter(filename.toUtf8().constData(),
                                   fourcc,
                                   fps,
                                   cv::Size(size.width(), size.height()));
}

VideoEncoder::~VideoEncoder()
{
    m_images.clear();
    m_running = false;
    delete m_writer;
    m_writer = 0;
}

bool VideoEncoder::isOpened() const
{
    return m_writer->isOpened();
}

void VideoEncoder::run()
{
    const int fromTo[] = { 0, 0, 1, 1, 2, 2 };
    while (m_running) {
        if (m_images.size() > 0) {
            const QImage &image = m_images.dequeue();
            uchar *data = const_cast<uchar *>(image.bits());
            cv::Mat mat(m_size.height(), m_size.width(), CV_8UC4, data, image.bytesPerLine());
            cv::Mat mat2(mat.rows, mat.cols, CV_8UC3);
            cv::mixChannels(&mat, 1, &mat2, 1, fromTo, 3);
            m_writer->write(mat2);
        }
    }
}

void VideoEncoder::enqueueImage(const QImage &image)
{
    m_images.enqueue(image);
}

void VideoEncoder::stop()
{
    m_running = false;
}
