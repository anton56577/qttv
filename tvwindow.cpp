#include <QtGui>
#include <QDesktopWidget>
#include <QApplication>
#include <X11/Xlib.h>
#include "tvwindow.h"
#include "tcontroller.h"

TVWindow::TVWindow() : QWidget()
{        
    setWindowFlags(windowType() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setMouseTracking (true);

    QPalette p (palette());
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);

    wndGeometry.setRect(0,0,320,240);
    this->setGeometry(0,0,320,240);
    iOldRect = this->geometry();

    TController::get()->SetWindowsWgt(this);

    tmpChan = -1;

    ActionChanlGroup = new QActionGroup(this);
    connect(ActionChanlGroup, SIGNAL(triggered(QAction*)), this, SLOT(change_chan(QAction*)));

    openGL = new GLWidget(this);
    TController::get()->SwitchTunerMode(TV);
    openGL->show();
    openGL->startShow();

    ParamsMenu = new QMenu(this);

    cw = new TVControlWidget(this);

    timerResetSaver = new QTimer(this);
    connect(timerResetSaver, SIGNAL(timeout()), this, SLOT(disablescreensaver()));
    timerResetSaver->start(60000);

    connect(&timerSwitchChan, SIGNAL(timeout()), this, SLOT(resetstateselchan()));

    TController::get()->SwitchChanNum(43);
}

TVWindow::~TVWindow()
{
    timerResetSaver->stop();
    openGL->stopShow();
    delete openGL;
    delete cw;
    TController::Uninit();
    qDebug("delete TVWindow");
}

void TVWindow::disablescreensaver()
{
    //блокировка скринсейвера
    Display* disp = XOpenDisplay(0);
    if (disp) {
        XResetScreenSaver(disp);
        XCloseDisplay(disp);
    }
 }

void TVWindow::SwitchToWindow()
{
    //переключение полный экран - обратно
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    setGeometry(iOldRect);
    //если меняли соотношение сторон, изменение для окна
    RecalcWndWidth(iOldRect.width());
}

void TVWindow::SwitchToFull()
{
    //переключение на полный экран
    iOldRect = geometry();
    setWindowState(windowState() | Qt::WindowFullScreen);
}

void TVWindow::resizeEvent (QResizeEvent * event)
{
    openGL->setFixedSize(event->size());
    event->accept();
}

void TVWindow::exit_tv()
{
    QCoreApplication::quit();
}

void TVWindow::closeEvent (QCloseEvent * event)
{
    event->accept();
}

void TVWindow::change_chan(QAction *selChanAct)
{
    TController::get()->SwitchChanNum(selChanAct->data().toInt());
}

void TVWindow::switch_scale_window(QAction *selScl)
{
    int newheight;
    TController::get()->ChangeScale((TScreenFormat) selScl->data().toInt(), width(), newheight);
    if (!TController::get()->isFullScrean())
        resize(width(), newheight);
}

void TVWindow::switchto_tv()
{
    TController::get()->SwitchTunerMode(TV);
}

void TVWindow::switchto_fm()
{
    TController::get()->SwitchTunerMode(FM);
}

void TVWindow::switchto_av()
{
    TController::get()->SwitchTunerMode(S_VIDEO);
}

void TVWindow::switchto_cs()
{
    TController::get()->SwitchTunerMode(COMPOSIT);
}

void TVWindow::mouseDoubleClickEvent (QMouseEvent * event)
{
    //двойной клик - на весь экран развернуть
    qDebug() << event->button();
    if (event->button() == Qt::LeftButton) {
        openGL->setCursor(Qt::ArrowCursor);
        TController::get()->SwitchFullScrean();
        event->accept();
    } else {
        event->ignore();
    }
}

void TVWindow::mouseMoveEvent (QMouseEvent * event)
{
    //разбивка окна по областям наведения мыши, если не на весь экран развернуто
    //qDebug("%d", event->pos().y());
    if (!TController::get()->isFullScrean()) {
        const int minw = 50;
        const int minh = 50;
        if (typeClk > 0 && (event->buttons () & Qt::LeftButton) == 1) {
            switch (typeClk) {
            case 1: {
                    //низ окна
                    openGL->setCursor(Qt::SizeVerCursor);
                    int newH = event->globalPos().y()-this->y();
                    if (newH > minh)
                        RecalcWndHeight(newH);
                    break;
                }
            case 2: {
                    //верх
                    openGL->setCursor(Qt::SizeVerCursor);
                    int newH = (wGlobpos.y() - event->globalPos().y())+wndGeometry.height();
                    if (newH > minh) {
                        this->move(this->x(), event->globalPos().y());
                        RecalcWndHeight(newH);
                    }
                    break;
                }
            case 3: {
                    //лево
                    openGL->setCursor(Qt::SizeHorCursor);
                    int newW = wGlobpos.x()-event->globalPos().x()+wndGeometry.width();
                    if (newW > minw) {
                        this->move(event->globalPos().x(), this->y());
                        RecalcWndWidth(newW);
                    }
                    break;
                }
            case 4: {
                    //право
                    this->setCursor(Qt::SizeHorCursor);
                    int newW = event->globalPos().x()-this->x();
                    if (newW > minw)
                        RecalcWndWidth(newW);
                    break;
                }
            case 5: {
                    //лево-верх
                    openGL->setCursor(Qt::SizeFDiagCursor);
                    int newH = (wGlobpos.y() - event->globalPos().y())+wndGeometry.height();

                    if (newH > minh) {
                        int newW;
                        TController::get()->ChangeWidthByScale(newW, newH);
                        this->move(wGlobpos.x()-newW+wndGeometry.width(), event->globalPos().y());
                        RecalcWndHeight(newH);
                    }

                    break;
                }
            case 6: {
                    //право-верх
                    openGL->setCursor(Qt::SizeBDiagCursor);

                    int newH = (wGlobpos.y() - event->globalPos().y())+wndGeometry.height();

                    if (newH > minh) {
                        RecalcWndHeight(newH);
                        this->move(this->x(), event->globalPos().y());
                    }
                    break;
                }
            case 7: {
                    //лево-низ
                    openGL->setCursor(Qt::SizeBDiagCursor);

                    int newW = wGlobpos.x()-event->globalPos().x()+wndGeometry.width();

                    if (newW > minw) {
                        RecalcWndWidth(newW);
                        this->move(event->globalPos().x(), this->y());
                    }
                    break;
                }
            case 8: {
                    //право-низ
                    openGL->setCursor(Qt::SizeFDiagCursor);
                    int newW = event->globalPos().x()-this->x();
                    if (newW > minw) {
                        RecalcWndWidth(newW);
                    }
                    break;
                }
            case 10:
                if ((event->buttons () & Qt::LeftButton) == 1 && typeClk == 10) {
                    //перетаскивание по левой + snap
                    //qDebug() << event->globalPos().x();
                    const int OFFSET = 20;
                    const QRect screen = qApp->desktop()->availableGeometry(this);

                    int leftpos = abs(event->globalPos().x() - wpos.x() - screen.left());
                    int rightpos = abs(event->globalPos().x() - wpos.x() + wndGeometry.width() - screen.right());
                    int toppos = abs(event->globalPos().y() - wpos.y() - screen.top());
                    int bottompos = abs(event->globalPos().y() - wpos.y() - screen.bottom() + wndGeometry.height());

                    if (leftpos < OFFSET && toppos > OFFSET && bottompos > OFFSET)
                        this->move(screen.left(),(event->globalPos() - wpos).y()); //left
                    else if(rightpos < OFFSET && toppos > OFFSET && bottompos > OFFSET)
                        this->move(screen.right() - wndGeometry.width() + 1,(event->globalPos() - wpos).y()); //right
                    else if(toppos < OFFSET && leftpos > OFFSET && rightpos > OFFSET)
                        this->move(event->globalPos().x() - wpos.x(), screen.top()); //top
                    else if(bottompos < OFFSET && leftpos > OFFSET && rightpos > OFFSET)
                        this->move(event->globalPos().x() - wpos.x(), screen.bottom()-wndGeometry.height() + 1); //bottom
                    else if (leftpos < OFFSET && toppos < OFFSET)
                        this->move(screen.left(), screen.top()); //left-top
                    else if (leftpos < OFFSET && bottompos < OFFSET)
                        this->move(screen.left(), screen.bottom()-wndGeometry.height() + 1); //left-bottom
                    else if (rightpos < OFFSET && toppos < OFFSET)
                        this->move(screen.right() - wndGeometry.width() + 1, screen.top()); //right-top
                    else if (rightpos < OFFSET && bottompos < OFFSET)
                        this->move(screen.right() - wndGeometry.width() + 1, screen.bottom()-wndGeometry.height() + 1); //right-bottom
                    else
                        this->move(event->globalPos() - wpos);
                }
                break;
            }
        } else {
            //наведение курсора без клика
            if (event->pos().y() >= this->height() - 10 && event->pos().x() > 10 && event->pos().x() < this->width() - 10) {
                //низ окна
                openGL->setCursor(Qt::SizeVerCursor);
            } else if (event->pos().y() <=  10 && event->pos().x() > 10 && event->pos().x() < this->width() - 10) {
                //верх
                openGL->setCursor(Qt::SizeVerCursor);
            } else if (event->pos().x() <= 10 && event->pos().y() >  10 && event->pos().y() < this->height() - 10) {
                //лево
                openGL->setCursor(Qt::SizeHorCursor);
            } else if (event->pos().x() >= this->width() - 10 && event->pos().y() >  10 && event->pos().y() < this->height() - 10) {
                //право
                openGL->setCursor(Qt::SizeHorCursor);
            } else if (event->pos().y() <=  10 && event->pos().x() <= 10) {
                //лево-верх
                openGL->setCursor(Qt::SizeFDiagCursor);
            } else if (event->pos().y() <=  10 && event->pos().x() >= this->width() - 10) {
                //право-верх
                openGL->setCursor(Qt::SizeBDiagCursor);
            } else if (event->pos().y() >= this->height() - 10 && event->pos().x() <= 10) {
                //лево-низ
                openGL->setCursor(Qt::SizeBDiagCursor);
            } else if (event->pos().y() >= this->height() - 10 && event->pos().x() >= this->width() - 10) {
                //право-низ
                openGL->setCursor(Qt::SizeFDiagCursor);
            } else {
                //перетаскивание - обычный курсор
                openGL->setCursor(Qt::ArrowCursor);
            }
        }
    } else {
        openGL->setCursor(Qt::ArrowCursor);
    }
    event->accept();
}

void TVWindow::mousePressEvent (QMouseEvent * event)
{
    //qDebug() << "mousePressEvent " << event->button();
    if (event->button() == Qt::RightButton) {
        //меню по правой кнопке
        ParamsMenu->clear();
        ParamsMenu->setTitle(trUtf8("Меню"));
        ParamsMenu->addAction(trUtf8("Окно"), this, SLOT(show_controlwidget()));
        ParamsMenu->addSeparator();

        QMenu *scaleMenu = ParamsMenu->addMenu(trUtf8("Соотношение сторон"));
        QActionGroup *ActionScaleGroup = new QActionGroup(this);
        connect(ActionScaleGroup, SIGNAL(triggered(QAction*)), this, SLOT(switch_scale_window(QAction*)));
        ActionScaleGroup->setExclusive(true);
        QAction *tmpAct4x3 = new QAction(trUtf8("4x3"), this);
        tmpAct4x3->setCheckable(true);
        tmpAct4x3->setData(0);
        QAction *tmpAct16x91 = new QAction(trUtf8("16x9 увеличить"), this);
        tmpAct16x91->setCheckable(true);
        tmpAct16x91->setData(1);
        QAction *tmpAct16x92 = new QAction(trUtf8("16x9 растянуть"), this);
        tmpAct16x92->setCheckable(true);
        tmpAct16x92->setData(2);
        QAction *tmpAct16x101 = new QAction(trUtf8("16x10 увеличить"), this);
        tmpAct16x101->setCheckable(true);
        tmpAct16x101->setData(3);
        QAction *tmpAct16x102 = new QAction(trUtf8("16x10 растянуть"), this);
        tmpAct16x102->setCheckable(true);
        tmpAct16x102->setData(4);
/*
 *        if (myChanls->GetThisChanl().iScreenFormat == frm4x3)
            tmpAct4x3->setChecked(true);
        else if (myChanls->GetThisChanl().iScreenFormat == frm16x9)
            tmpAct16x91->setChecked(true);
        else if (myChanls->GetThisChanl().iScreenFormat == frm16x9pan)
            tmpAct16x92->setChecked(true);
        else if (myChanls->GetThisChanl().iScreenFormat == frm16x10)
            tmpAct16x101->setChecked(true);
        else if (myChanls->GetThisChanl().iScreenFormat == frm16x10pan)
            tmpAct16x102->setChecked(true);
*/
        ActionScaleGroup->addAction(tmpAct4x3);
        ActionScaleGroup->addAction(tmpAct16x91);
        ActionScaleGroup->addAction(tmpAct16x92);
        ActionScaleGroup->addAction(tmpAct16x101);
        ActionScaleGroup->addAction(tmpAct16x102);
        scaleMenu->addAction(tmpAct4x3);
        scaleMenu->addAction(tmpAct16x91);
        scaleMenu->addAction(tmpAct16x92);
        scaleMenu->addAction(tmpAct16x101);
        scaleMenu->addAction(tmpAct16x102);

        ParamsMenu->addSeparator();
        ParamsMenu->addAction(trUtf8("FM"), this, SLOT(switchto_fm()));
        ParamsMenu->addAction(trUtf8("TV"), this, SLOT(switchto_tv()));
        ParamsMenu->addAction(trUtf8("S-VIDEO"), this, SLOT(switchto_av()));
        ParamsMenu->addAction(trUtf8("COMPOSITE"), this, SLOT(switchto_cs()));

        ParamsMenu->addSeparator();
        ParamsMenu->addAction(trUtf8("Настройки"), this, SLOT(show_propdailog()));

        ParamsMenu->addSeparator();
        ParamsMenu->addAction(trUtf8("Выход"), this, SLOT(exit_tv()));

        ParamsMenu->exec(event->globalPos());

        event->accept();
    } else if (event->button() == Qt::MiddleButton && event->modifiers() == Qt::NoModifier) {
        //список каналов по средней кнопке
        ParamsMenu->clear();
        for (int i=0; i<ActionChanlGroup->actions().count();i++) {
            QAction *tmpAct = ActionChanlGroup->actions()[i];
            delete tmpAct;
        }
        ActionChanlGroup->actions().clear();
        qDebug("cnt=%d", ActionChanlGroup->actions().count());
        ParamsMenu->setTitle(trUtf8("Каналы"));
        TController::get()->AddChannelsToMenu(ParamsMenu, ActionChanlGroup, this);
        ActionChanlGroup->setExclusive(true);
        //ParamsMenu->popup(event->globalPos());
        ParamsMenu->exec(mapToGlobal(event->pos()));

        event->accept();

    } else if (event->button() == Qt::MiddleButton && event->modifiers() == Qt::ControlModifier) {
        //средняя кнопка + ctrl - mute
        TController::get()->VolimeMute();

        event->accept();

    } else if (event->button() == Qt::LeftButton && !TController::get()->isFullScrean()) {
        //запоминаем позицию куда нажали левой кнопкой
        wpos = event->pos();
        wGlobpos = event->globalPos();
        wndGeometry = this->geometry();
        if (event->pos().y() >= this->height() - 10 && event->pos().x() > 10 && event->pos().x() < this->width() - 10) {
            openGL->setCursor(Qt::SizeVerCursor);
            typeClk = 1; //изменение размера за низ окна
        } else if (event->pos().y() <=  10 && event->pos().x() > 10 && event->pos().x() < this->width() - 10) {
            openGL->setCursor(Qt::SizeVerCursor);
            typeClk = 2; //изменение размера за верх
        } else if (event->pos().x() <= 10 && event->pos().y() >  10 && event->pos().y() < this->height() - 10) {
            openGL->setCursor(Qt::SizeHorCursor);
            typeClk = 3; //изменение размера за лево
        } else if (event->pos().x() >= this->width() - 10 && event->pos().y() >  10 && event->pos().y() < this->height() - 10) {
            openGL->setCursor(Qt::SizeHorCursor);
            typeClk = 4; //изменение размера за право
        } else if (event->pos().y() <=  10 && event->pos().x() <= 10) {
            openGL->setCursor(Qt::SizeFDiagCursor);
            typeClk = 5; //изменение размера за лево-верх
        } else if (event->pos().y() <=  10 && event->pos().x() >= this->width() - 10) {
            openGL->setCursor(Qt::SizeBDiagCursor);
            typeClk = 6; //изменение размера за право-верх
        } else if (event->pos().y() >= this->height() - 10 && event->pos().x() <= 10) {
            openGL->setCursor(Qt::SizeBDiagCursor);
            typeClk = 7; //изменение размера за лево-низ
        } else if (event->pos().y() >= this->height() - 10 && event->pos().x() >= this->width() - 10) {
            openGL->setCursor(Qt::SizeFDiagCursor);
            typeClk = 8; //изменение размера за право-низ
        } else {
            typeClk = 10; //перетаскивание
        }

        event->accept();
    } else {
        event->ignore();
    }
}

void TVWindow::mouseReleaseEvent (QMouseEvent * event)
{
    typeClk = 0;
    QApplication::restoreOverrideCursor();
    event->accept();
}

void TVWindow::wheelEvent(QWheelEvent *event)
{
    //event->delta();
    //qDebug("%d-%d", event->pos().x(), event->pos().y());
    if (event->modifiers() == Qt::ControlModifier){
        //каналы по колесику мыши + ctrl
        if (event->delta() > 0) {
            //вперед
            TController::get()->SwitchChanNext();
        } else  {
            //канал назад
            TController::get()->SwitchChanBack();
        }
    }else {
        //громкость по колесику мыши
        if (event->delta() > 0)
            TController::get()->VolimeUp();
        else
            TController::get()->VolimeDown();
    }
    event->accept();
}

void TVWindow::show_propdailog()
{
    TController::get()->ShowPropDialog();
}

void TVWindow::show_controlwidget()
{
    //управление
    cw->show();
}

void TVWindow::keyPressEvent(QKeyEvent *event)
{
     qDebug() << event->key() << " - " << event->nativeScanCode() << " - " << event->nativeModifiers();
    //переключение по клавиатуре
    if (event->key() == Qt::Key_Up) {
        //канал вперед
        TController::get()->SwitchChanNext();
    } else if (event->key() == Qt::Key_Down) {
        //канал назад
        TController::get()->SwitchChanBack();
    } else if (event->key() == Qt::Key_Backspace) {
        //предыдущий канал
        TController::get()->SwitchChanPrev();
    } else if (event->key() == Qt::Key_Left) {
        //громкость ниже
        TController::get()->VolimeDown();
    } else if (event->key() == Qt::Key_Right) {
        //громкость выше
        TController::get()->VolimeUp();
    } else if (event->nativeScanCode() == 58) { //M
        //громкость on\off
        TController::get()->VolimeMute();
    } else if (event->nativeScanCode() == 41 || (event->key() == 16777220 && event->nativeModifiers() == 24)) { //F or alt+enter
        //полный экран
        TController::get()->SwitchFullScrean();
    } else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        //цифра - переключние канала
        if (tmpChan == -1) {
            //первая цифра
            tmpChan = event->key() - 48;
            //таймер для второй цифры
            timerSwitchChan.start(1500);
        } else {
            //вторая цифра
            int cnl = tmpChan * 10 + event->key() - 48;
            TController::get()->SwitchChanNum(cnl - 1);
            tmpChan = -1;
            timerSwitchChan.stop();
        }
    }
}

void TVWindow::resetstateselchan()
{
    //переход по таймеру на первой цифре
    TController::get()->SwitchChanNum(tmpChan - 1);
    tmpChan = -1;
    timerSwitchChan.stop();
}

void TVWindow::RecalcWndWidth(const int wndWidth)
{
    if (!TController::get()->isFullScrean()) {
        int newH;
        TController::get()->ChangeHeightByScale(wndWidth, newH);
        resize(wndWidth, newH);
    }
}

void TVWindow::RecalcWndHeight(const int wndHeight)
{
    if (!TController::get()->isFullScrean()) {
        int newW;
        TController::get()->ChangeWidthByScale(newW, wndHeight);
        resize(newW, wndHeight);
    }
}
