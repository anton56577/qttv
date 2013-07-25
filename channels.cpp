#include <QObject>
#include <QSettings>
#include <QApplication>
#include <QTextCodec>
#include <QtDebug>
#include "channels.h"

TChannels::TChannels()
{
    chanslist.clear();
    ThisChan = chanslist.end();
    PrevChan = chanslist.end();
}

TChannels::~TChannels()
{
    chanslist.clear();
}

void TChannels::ReloadChanlList(const TChanType ChanType)
{
    //загрузка списка каналов в список
    chanslist.clear();
    ThisChan = chanslist.end();
    PrevChan = chanslist.end();
    if (ChanType == FM) {
        QSettings chanini(QApplication::applicationDirPath()+"/fmchan.ini", QSettings::IniFormat);
        QTextCodec *codec = QTextCodec::codecForName("WINDOWS-1251");
        chanini.setIniCodec(codec);
        for (int i=0; i<10000; i++) {
            chanini.beginGroup("FM"+QString::number(i+1));
            int fnd = chanini.value("Frequency", -1).toInt();
            if (fnd > 0) {
                TChanParams tmpcn;
                tmpcn.id = i;
                tmpcn.sName = chanini.value("Name", "").toString().toStdString();
                tmpcn.iFreqHz = chanini.value("Frequency", 0).toInt();
                tmpcn.iSkip = 0;
                tmpcn.iAudioMode = chanini.value("FMAudioMode", 0).toInt();
                chanslist.push_back(tmpcn);
            } else {
                break;
            }
            chanini.endGroup();
        }
    } else if (ChanType == TV) {
        QSettings chanini(QApplication::applicationDirPath()+"/tvchan.ini", QSettings::IniFormat);
        QTextCodec *codec = QTextCodec::codecForName("WINDOWS-1251");
        chanini.setIniCodec(codec);
        for (int i=0; i<10000; i++) {
            chanini.beginGroup("TV"+QString::number(i+1));
            // или inifile.value("Имя группы/имя параметра");
            int fnd = chanini.value("TVFrequency", -1).toInt();
            if (fnd > 0) {
                TChanParams tmpcn;
                tmpcn.id = i;
                tmpcn.sName = chanini.value("Name", "").toString().toStdString();
                tmpcn.iFreqHz = chanini.value("TVFrequency", 0).toInt();
                tmpcn.iSkip = 0;
                tmpcn.iVideoMode = chanini.value("VideoStand", 0).toInt();
                tmpcn.iAudioMode = chanini.value("AudioMode", 0).toInt();
                tmpcn.iAudioChannel = chanini.value("AudioChannel", 0).toInt();
                tmpcn.iScreenFormat = (TScreenFormat)0;
                chanslist.push_back(tmpcn);
            } else {
                break;
            }
            chanini.endGroup();
        }
    }
}

const TChanParams &TChannels::GetThisChanl() const
{
    return *ThisChan;
}

const TChanParams &TChannels::SwitchToChanl(int NumChan)
{
    //switch chan
    for (std::vector<TChanParams>::iterator itChn = chanslist.begin(); itChn != chanslist.end(); ++itChn) {
        if (itChn->id == NumChan) {
            if (ThisChan != chanslist.end())
                PrevChan = ThisChan;
            else
                PrevChan = itChn;
            ThisChan = itChn;
            break;
        }
    }
    //config ini save currchan...

    return *ThisChan;
}

const TChanParams &TChannels::SwitchToNextChanl()
{
    if (ThisChan->id < (int)chanslist.size() - 1)
        SwitchToChanl((++ThisChan)->id);
    else
        SwitchToChanl(0);
    return *ThisChan;
}

const TChanParams &TChannels::SwitchToBackChanl()
{
    if (ThisChan->id > 0)
        SwitchToChanl((--ThisChan)->id);
    else
        SwitchToChanl(chanslist.size() - 1);
    return *ThisChan;
}

const TChanParams &TChannels::SwitchToPrevChanl()
{
    if (PrevChan->id > 0 && PrevChan->id < (int)chanslist.size())
        SwitchToChanl(PrevChan->id);
    return *ThisChan;
}

int TChannels::GetChanlsCount() const
{
    return chanslist.size();
}

std::vector<std::string> TChannels::GetChanelsNames() const
{
    std::vector<std::string> chnnms;
    chnnms.reserve(chanslist.size());
    for (std::vector<TChanParams>::const_iterator itChn = chanslist.begin(); itChn != chanslist.end(); ++itChn) {
        chnnms.push_back(itChn->sName);
    }
    return chnnms;
}

int TChannels::SetThisChanlScreenFromat(const TScreenFormat ScreenFormat)
{
    ThisChan->iScreenFormat = ScreenFormat;
    //ini save...
    return 0;
}
