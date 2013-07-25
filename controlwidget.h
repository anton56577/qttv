#ifndef TVCONTROLWIDGET_H
#define TVCONTROLWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
    class TVControlWidget;
}

class TVControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TVControlWidget(QWidget *parent = 0);
    ~TVControlWidget();

private:
    Ui::TVControlWidget *ui;

private slots:
    void on_pushButton_10_clicked();
    void animate_clock();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_verticalSlider_sliderMoved(int position);
};

#endif // TVCONTROLWIDGET_H
