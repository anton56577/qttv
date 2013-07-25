#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QSettings>

namespace Ui {
    class PropertiesDialog;
}

class PropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertiesDialog(QWidget *parent = 0);
    ~PropertiesDialog();
    static bool isShowNow;

private:
    Ui::PropertiesDialog *ui;

    void LoadPropFromFile();
    void LoadParamsConf();
    void LoadTVChans();
    void LoadFMChans();
    QSettings *tvchanini;
    QSettings *fmchanini;
    void LoadChanDataToEdit(int index);


private slots:
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem* item);
    void on_tableWidget_itemClicked(QTableWidgetItem* item);
};

#endif // PROPERTIESDIALOG_H
