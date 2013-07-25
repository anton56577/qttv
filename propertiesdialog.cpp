#include <QStringList>
#include <QTextCodec>
#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

#include "channels.h"
#include "tvwindow.h"
#include "tcontroller.h"

const char* sVideoModeName[] = {"Auto", "SECAM", "PAL", "NTSC"};
const int VideoModeNameSz = 4;
const char* sAudioModeName[] = {"BG SECAM/PAL", "DK SECAM/PAL", "I PAL", "L SECAM", "N PAL", "H PAL", "M NTCS/PAL"};
const int AudioModeNameSz = 7;
const char* sAudioChanName[] = {"Mono", "Stereo", "Language1", "Language2"};
const int AudioChanNameSz = 4;

bool PropertiesDialog::isShowNow = 0;

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog)
{
    PropertiesDialog::isShowNow = 1;
    ui->setupUi(this);

    ui->comboBox->clear();
    for (int i = 0; i < 4; i++)
        ui->comboBox->addItem(sVideoModeName[i], i);

    ui->comboBox_2->clear();
    for (int i = 0; i < 7; i++)
        ui->comboBox_2->addItem(sAudioModeName[i], i);

    ui->comboBox_3->clear();
    for (int i = 0; i < 4; i++)
        ui->comboBox_3->addItem(sAudioChanName[i], i);

    tvchanini = new QSettings(QApplication::applicationDirPath()+"/tvchan.ini", QSettings::IniFormat, this);
    QTextCodec *codec = QTextCodec::codecForName("WINDOWS-1251");
    tvchanini->setIniCodec(codec);
    fmchanini = new QSettings(QApplication::applicationDirPath()+"/fmchan.ini", QSettings::IniFormat, this);
    fmchanini->setIniCodec(codec);

    //загрузка параметров в форму
    LoadParamsConf();
    LoadTVChans();
    LoadFMChans();
}

PropertiesDialog::~PropertiesDialog()
{
    delete ui;
    PropertiesDialog::isShowNow = 0;
}

void PropertiesDialog::LoadPropFromFile()
{
    //загрузка настроек
    //QSettings settings;
}

void PropertiesDialog::LoadParamsConf()
{
    //
}

void PropertiesDialog::LoadTVChans()
{
    //список TV каналов

    ui->tableWidget->clear();
    ui->tableWidget->setUpdatesEnabled(false);
    //ui->tableWidget->verticalHeader()->hide();

    const char headTbl[8][30] = {"", "№", "Название", "Частота", "Видео", "Аудио", "Канал", "Громкость"};
    ui->tableWidget->setColumnCount(8);
    for (int i=0; i < 8; i++) {
        QTableWidgetItem *newItem = new QTableWidgetItem(trUtf8(headTbl[i]));
        ui->tableWidget->setHorizontalHeaderItem(i, newItem);
    }

    for (int i=0; i<10000; i++) {
        tvchanini->beginGroup("TV" + QString::number(i+1));
        // или inifile.value("Имя группы/имя параметра");
        int fnd = tvchanini->value("TVFrequency", -1).toInt();
        if (fnd > 0) {

            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

            QTableWidgetItem *chkBx = new QTableWidgetItem(QTableWidgetItem::UserType);
            chkBx->setData(Qt::CheckStateRole, "");
            chkBx->setCheckState(Qt::Checked);
            ui->tableWidget->setItem(i,0, chkBx);
            QTableWidgetItem *NumCh = new QTableWidgetItem(QString::number(i+1));
            ui->tableWidget->setItem(i,1, NumCh);
            QTableWidgetItem *sName = new QTableWidgetItem(QString(tvchanini->value("Name", "").toByteArray()));
            ui->tableWidget->setItem(i,2, sName);
            QTableWidgetItem *FreqHz = new QTableWidgetItem(QString::number(tvchanini->value("TVFrequency", 0).toInt()/1000000.00));
            ui->tableWidget->setItem(i,3, FreqHz);
            if (tvchanini->value("VideoStand", 0).toInt() < VideoModeNameSz) {
                QTableWidgetItem *VidMode = new QTableWidgetItem(sVideoModeName[tvchanini->value("VideoStand", 0).toInt()]);
                ui->tableWidget->setItem(i,4, VidMode);
            }
            if (tvchanini->value("AudioMode", 0).toInt() < AudioModeNameSz) {
                QTableWidgetItem *AudMode = new QTableWidgetItem(sAudioModeName[tvchanini->value("AudioMode", 0).toInt()]);
                ui->tableWidget->setItem(i,5, AudMode);
            }
            if (tvchanini->value("AudioChannel", 0).toInt() < AudioChanNameSz) {
                QTableWidgetItem *AudChan = new QTableWidgetItem(sAudioChanName[tvchanini->value("AudioChannel", 0).toInt()]);
                ui->tableWidget->setItem(i,6, AudChan);
            }
        } else {
            i = 10000;
        }
        tvchanini->endGroup();
    }
    //ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();

//    ui->tableWidget->setCurrentCell(channels::selchanid, 0);

    ui->tableWidget->setUpdatesEnabled(true);

//    LoadChanDataToEdit(channels::selchanid);
//    ui->tableWidget->setFocus();

}

void PropertiesDialog::LoadFMChans()
{
//    for (int i=0; i<10000; i++) {
//        chanini.beginGroup("FM"+QString::number(i+1));
//       int fnd = chanini.value("Frequency", -1).toInt();
//       if (fnd > 0) {
//            chanparam tmpcn;
//           tmpcn.id = i+1;
//            tmpcn.sName = QObject::trUtf8(chanini.value("Name", "").toByteArray());
//            tmpcn.iFreqHz = chanini.value("Frequency", 0).toInt();
//            tmpcn.iSkip = 0;
//            tmpcn.iAudioMode = chanini.value("FMAudioMode", 0).toInt();
//            chanslist.push_back(tmpcn);
//        } else {
//            i = 10000;
//        }
//        chanini.endGroup();
//    }

}

void PropertiesDialog::on_tableWidget_itemClicked(QTableWidgetItem* item)
{
    //клик на таблице ТВ каналов
    if (item->column() == 0) {
        //галка
        //qDebug("%d", item->checkState());
    }

    LoadChanDataToEdit(item->row());
}

void PropertiesDialog::LoadChanDataToEdit(int index)
{
    //загрузка данных канала

    tvchanini->beginGroup("TV" + QString::number(index+1));
    int fnd = tvchanini->value("TVFrequency", -1).toInt();
    if (fnd > 0) {
        ui->lineEdit_4->setText(trUtf8(tvchanini->value("Name", "").toByteArray()));
        ui->doubleSpinBox->setValue(tvchanini->value("TVFrequency", 0).toInt()/1000000.00);
        ui->comboBox->setCurrentIndex(tvchanini->value("VideoStand", 0).toInt());
        ui->comboBox_2->setCurrentIndex(tvchanini->value("AudioMode", 0).toInt());
        ui->comboBox_3->setCurrentIndex(tvchanini->value("AudioChannel", 0).toInt());
    } else {
        //Не найден ?)
    }

    tvchanini->endGroup();
}

void PropertiesDialog::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    //двойной клик - переключение канала
    TController::get()->SwitchChanNum(item->row());
}

void PropertiesDialog::on_pushButton_4_clicked()
{
    //отмена
    close();
}

void PropertiesDialog::on_pushButton_3_clicked()
{
    //сохранить
    close();
}

