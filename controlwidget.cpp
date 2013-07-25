#include <QtGui>
#include "controlwidget.h"
#include "ui_controlwidget.h"
#include "propertiesdialog.h"
#include "tcontroller.h"

TVControlWidget::TVControlWidget(QWidget *parent) :
    QWidget(parent, Qt::Tool),
    ui(new Ui::TVControlWidget)
{
    ui->setupUi(this);

    TController::get()->SetVolSlider(this->ui->verticalSlider, this->ui->pushButton_9);

    QTimer *timerclock = new QTimer(this);
    //v_timerclock = timerclock;
    connect(timerclock, SIGNAL(timeout()), this, SLOT(animate_clock()));
    timerclock->start(500);
}

TVControlWidget::~TVControlWidget()
{
    //delete (QTimer*)v_timerclock;
    delete ui;
}

void TVControlWidget::on_pushButton_10_clicked()
{
    //настройки
    TController::get()->ShowPropDialog();
}

void TVControlWidget::animate_clock()
{
    QTime time = QTime::currentTime();
    ui->label_3->setText(time.toString("H:mm:ss"));
}

void TVControlWidget::on_pushButton_5_clicked()
{
    //канал ниже
    TController::get()->SwitchChanNext();
}

void TVControlWidget::on_pushButton_6_clicked()
{
    //канал выше
    TController::get()->SwitchChanBack();
}

void TVControlWidget::on_pushButton_7_clicked()
{
    //громкость выше
    TController::get()->VolimeUp();
}

void TVControlWidget::on_pushButton_8_clicked()
{
    //громкость ниже
    TController::get()->VolimeDown();
}

void TVControlWidget::on_pushButton_9_clicked()
{
    TController::get()->VolimeMute();
}

void TVControlWidget::on_verticalSlider_sliderMoved(int position)
{
    TController::get()->VolimeSet(position);
}
