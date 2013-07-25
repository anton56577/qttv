#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QWidget>
#include "v4l2capiture.h"
#include "channels.h"

class PicShowThread;
QT_BEGIN_NAMESPACE
class QPaintEvent;
class QWidget;
QT_END_NAMESPACE

class GLWidget : public QGLWidget
{
    Q_OBJECT
    void mouseMoveEvent (QMouseEvent * event);
    void mouseReleaseEvent (QMouseEvent * event);
    void mousePressEvent (QMouseEvent * event);
    void mouseDoubleClickEvent (QMouseEvent * event);
    void wheelEvent(QWheelEvent *event);
//    void resizeEvent (QResizeEvent * event);

protected:
    bool doResize;
    bool doRendering;
    int iMode;
    int iScrFormat;
    QSize whGl;
    QTimer *timerpaint;    
    QWidget *parentwgt;

    v4l2capiture v4l2;

    void Deinterlace();

    void resizeEvent (QResizeEvent * event);
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

public:
    GLWidget(QWidget *parent);
    ~GLWidget();
    void startShow();
    void stopShow();
    void switchMode(int mode);
    void switchScreenFrormat(int frmt);
    void SwitchToChan(const TChanParams &thisChan);//    //const TChanParams &thisChan = myChanls.SwitchToChanl(idchn);

public slots:
    void animate();
};

#endif
