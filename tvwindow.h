#ifndef TVWINDOW_H
#define TVWINDOW_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include "glwidget.h"
#include "controlwidget.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QWidget;
QT_END_NAMESPACE

class TVWindow : public QWidget
{
    Q_OBJECT
    void closeEvent (QCloseEvent * event);
    void resizeEvent (QResizeEvent * event);

    int tmpChan;
    QTimer timerSwitchChan;

    int typeClk;

    QRect iOldRect;

    QMenu *ParamsMenu;

    QPoint wpos;
    QPoint wGlobpos;
    QRect wndGeometry;
    QActionGroup *ActionChanlGroup;
    QTimer *timerResetSaver;

    TVControlWidget *cw;

    void SwitchToFullScr();

public:    
    TVWindow();
    ~TVWindow();

    void keyPressEvent (QKeyEvent * event);

    void mouseMoveEvent (QMouseEvent * event);
    void mouseReleaseEvent (QMouseEvent * event);
    void mousePressEvent (QMouseEvent * event);
    void mouseDoubleClickEvent (QMouseEvent * event);
    void wheelEvent(QWheelEvent *event);

    void RecalcWndWidth(const int wndWidth);
    void RecalcWndHeight(const int wndHeight);

    void SwitchToWindow();
    void SwitchToFull();

    GLWidget *openGL;

private slots:
     void exit_tv();
     void show_propdailog();
     void show_controlwidget();
     void change_chan(QAction*);
     void disablescreensaver();
     void switch_scale_window(QAction *selScl);
     void switchto_tv();
     void switchto_fm();
     void resetstateselchan();
};

#endif
