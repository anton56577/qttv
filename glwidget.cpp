#include <QtGui>
#include <QGLFormat>
#include "glwidget.h"
#include "tvwindow.h"
#include <stdint.h>
//#define UINT64_C(value)   __CONCAT(value, ULL)
#include "libavcodec/avcodec.h"

GLWidget::GLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), parentwgt(parent)
{  

    setWindowFlags(windowType() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    //setAttribute(Qt::WA_PaintOutsidePaintEvent, true);

    setAutoBufferSwap(false);
    setMouseTracking (true);
    setAutoFillBackground(false);
    v4l2.bufTV = NULL;
    v4l2.sizebuf = 0;
    v4l2.start("/dev/video1", 2);
    setFixedSize(320, 240);

    //ShowThread = new PicShowThread(this);
    timerpaint = new QTimer(this);
    connect(timerpaint, SIGNAL(timeout()), this, SLOT(animate()));
    timerpaint->start(25);

    doRendering = true;
    doResize = false;
    iMode = 0;
    iScrFormat = 0;
}

GLWidget::~GLWidget()
{
    timerpaint->stop();
/*    if (ShowThread) {
        stopShow();
        //ShowThread->quit();
    }
    //if (ShowThread)
    //    delete ShowThread;

*/
    v4l2.end();
    qDebug("delete GLWidget");
}

/*void GLWidget::Deinterlace()
{
    //http://mediaservers.ru/mediaservers/136-ispolzovanie-libavformat-i-libavcodec.html
    PixelFormat pix_fmt = PIX_FMT_RGBA;
    AVPicture *src = NULL;
    AVPicture *dst = NULL;
    avpicture_alloc(src, pix_fmt, v4l2.pwidth, v4l2.pheigth);
    avpicture_alloc(dst, pix_fmt, v4l2.pwidth, v4l2.pheigth);
    avpicture_fill(src, (uint8_t*)v4l2.sizebuf, pix_fmt, v4l2.pwidth, v4l2.pheigth);
    int x = avpicture_deinterlace(dst, src, pix_fmt, v4l2.pwidth, v4l2.pheigth);

    //memcpy(v4l2.bufTV, dst->data, v4l2.sizebuf);
    avpicture_free(src);
    avpicture_free(dst);

}
*/

///*
void GLWidget::Deinterlace()
{
    //Field-mode combing detection conceived by Gunnar Thalin.
    long bpitch, dblbpitch;
    int x, y;
    uchar *src, *srcminus, *srcplus;
    long gr, gg, gb, gt;
    const long threshold = 40 * 40;

    bpitch = 4 * v4l2.pwidth;
    dblbpitch = bpitch * 2;

    // Calculate
    src = v4l2.bufTV + bpitch;
    srcminus = v4l2.bufTV;
    srcplus = v4l2.bufTV + dblbpitch;
    for (y = 1; y < v4l2.pheigth-1; y += 2) {
        for (x = 0; x < bpitch; x += 4) {
            gb =     ((srcminus[x+2] - src[x+2]) * (srcplus[x+2]  - src[x+2]));
            gg = 6 * ((srcminus[x+1] - src[x+1]) * (srcplus[x+1] - src[x+1]));
            gr = 3 * ((srcminus[x] - src[x]) * (srcplus[x]  - src[x]));
            gt = (gr + gg + gb);

            if (gt > threshold) {
                src[x] =   (srcminus[x] + srcplus[x]) >> 1;
                src[x+1] = (srcminus[x+1] + srcplus[x+1]) >> 1;
                src[x+2] = (srcminus[x+2] + srcplus[x+2]) >> 1;
                //src[x] = 255; //B
                //src[x+1] = 0; //G
                //src[x+2] = 0; //R
            }
        }
        src += dblbpitch;
        srcminus += dblbpitch;
        srcplus += dblbpitch;
    }

    //free(bits);
}
//*/

void GLWidget::paintEvent(QPaintEvent *)
{
    ////Рисуется в потоке

    makeCurrent();
    //while(doRendering) {
    if (doRendering && iMode != 3) {

        if (doResize) {
            setFixedSize(whGl);
            doResize = false;
        }

        v4l2.mainloop();
        if (v4l2.bufTV && v4l2.sizebuf > 0) {

            Deinterlace();

            int imgNewH = height();            
            int imgNewW = imgNewH * 4 / 3;

            int imgNewX = (width() - imgNewW) / 2;
            int imgNewY = 0;

            if (iScrFormat == 1) {
                imgNewH = height();
                imgNewW = imgNewH * 16 / 9;
                imgNewH = imgNewW * 3 / 4;

                imgNewX = (width() - imgNewW) / 2;
                imgNewY = (height() - imgNewH) / 2;

            } else if (iScrFormat == 2) {
                imgNewH = height();
                imgNewW = imgNewH * 16 / 9;
                imgNewX = 0;
            } else if (iScrFormat == 3) {
                imgNewH = height();                
                imgNewW = imgNewH * 16 / 10;
                imgNewH = imgNewW * 3 / 4;

                imgNewX = (width() - imgNewW) / 2;
                imgNewY = (height() - imgNewH) / 2;

            } else if (iScrFormat == 4) {
                imgNewH = height();
                imgNewW = imgNewH * 16 / 10;
                imgNewX = 0;
            }


            QPainter painter;
            QBrush background = QBrush(QColor(0, 0, 0));

            painter.begin(this);

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            //fon
            painter.fillRect(QRect(0, 0, whGl.width(), whGl.height()), background);

///*            //TV image
            QImage imagebuf(v4l2.bufTV, v4l2.pwidth, v4l2.pheigth, QImage::Format_RGB32);
            painter.drawImage(QRect(imgNewX, imgNewY, imgNewW, imgNewH), imagebuf, QRect(0, 0, v4l2.pwidth, v4l2.pheigth), 0);
            //painter.drawImage(0, 0, imagebuf);
            //osd
            painter.setPen(Qt::green);
            painter.setFont(QFont("Arial",10*imgNewW/v4l2.pwidth));
            painter.drawText(QRect(0, 0, 50, 50), Qt::AlignCenter, "TV");//parentWnd->selchanid
//*/
            /*painter.beginNativePainting();
            //TV image
            glEnable(GL_TEXTURE_2D); // включаем текстурирование
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);         // выравнивание
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0,
                         GL_RGBA, // внутренний формат RGB
                         v4l2.pwidth, v4l2.pheigth, // размер текстуры
                         0, // рамка, сейчас никто не использует
                         GL_BGRA, // формат пикселей RGB
                         GL_UNSIGNED_BYTE, // каждая компонента - байт
                         v4l2.bufTV); // указатель на пиксели
            glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(0.0+imgNewX, imgNewH, 0.0);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(imgNewW+imgNewX, imgNewH, 0.0);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(imgNewW+imgNewX, 0, 0.0);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(0+imgNewX, 0, 0.0);
            glEnd();

            glDisable(GL_TEXTURE_2D);

            painter.endNativePainting();*/

            painter.end();

            swapBuffers();
            //msleep(10);
        }
    }

}

void GLWidget::animate()
{
    repaint();
}

void GLWidget::startShow()
{
    //ShowThread->resume();
    //ShowThread->start();
    timerpaint->start();
    doRendering = true;
}

void GLWidget::stopShow()
{
    //ShowThread->stop();
    //ShowThread->wait();
    doRendering = false;
    timerpaint->stop();
}

void GLWidget::resizeEvent (QResizeEvent * event)
{
    //ShowThread->resizeView(event->size());
    doRendering = false;
    whGl.setWidth(event->size().width());
    whGl.setHeight(event->size().height());
    doRendering = true;
}

void GLWidget::closeEvent(QCloseEvent *event)
{
    stopShow();
    QGLWidget::closeEvent(event);
    qDebug("closeEvent");
}

void GLWidget::switchMode(int mode)
{
    stopShow();
    v4l2.end();
    iMode = mode;
    if (mode == 3)
        v4l2.start("/dev/radio0", mode);
    else
        v4l2.start("/dev/video1", mode);
    startShow();
}

void GLWidget::switchScreenFrormat(int frmt)
{
    iScrFormat = frmt;
}

void GLWidget::SwitchToChan(const TChanParams &thisChan)
{
    switchScreenFrormat(thisChan.iScreenFormat);
    v4l2.selTvChannel(thisChan.iFreqHz, thisChan.iVideoMode, thisChan.iAudioMode);
}

void GLWidget::mouseMoveEvent (QMouseEvent * event)
{
 static_cast<TVWindow *>(parentwgt)->mouseMoveEvent(event);
}

void GLWidget::mouseReleaseEvent (QMouseEvent * event)
{
 static_cast<TVWindow *>(parentwgt)->mouseReleaseEvent(event);
}

void GLWidget::mousePressEvent (QMouseEvent * event)
{
 static_cast<TVWindow *>(parentwgt)->mousePressEvent(event);
}

void GLWidget::mouseDoubleClickEvent (QMouseEvent * event)
{
 static_cast<TVWindow *>(parentwgt)->mouseDoubleClickEvent(event);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
 static_cast<TVWindow *>(parentwgt)->wheelEvent(event);
}
