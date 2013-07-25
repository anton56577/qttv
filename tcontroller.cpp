#include <string>
#include <QAbstractButton>
#include "tcontroller.h"
#include "propertiesdialog.h"

TController *TController::ct = NULL;

TController::TController()
    : windowWgt(NULL),
      sliderVol(NULL),
      volumeCtrl(NULL),
      myChanls(NULL),
      volMuteBtn(NULL),
      iFullScreen(0)
{
    volumeCtrl = new TAudioControl();
    myChanls = new TChannels();
}

TController::~TController()
{
    delete volumeCtrl;
    delete myChanls;
    windowWgt = NULL;
    sliderVol = NULL;
}

TController *TController::get()
{
    if (!ct)
        ct = new TController();
    return ct;
}

void TController::Uninit()
{
    if (ct)
        delete ct;
    ct = NULL;
}

void TController::SetWindowsWgt(void *windowWidget)
{
    windowWgt = windowWidget;
}

void TController::SwitchChanNum(const int chan) const
{
    if (chan >= 0 && chan < myChanls->GetChanlsCount()) {
        ((TVWindow *)windowWgt)->openGL->SwitchToChan(myChanls->SwitchToChanl(chan));
    }
}

void TController::SwitchChanNext() const
{
    ((TVWindow *)windowWgt)->openGL->SwitchToChan(myChanls->SwitchToNextChanl());
}

void TController::SwitchChanPrev() const
{
    ((TVWindow *)windowWgt)->openGL->SwitchToChan(myChanls->SwitchToPrevChanl());
}

void TController::SwitchChanBack() const
{
    ((TVWindow *)windowWgt)->openGL->SwitchToChan(myChanls->SwitchToBackChanl());
}

void TController::SwitchTunerMode(const TChanType cnmode) const
{
    myChanls->ReloadChanlList(cnmode);
    ((TVWindow *)windowWgt)->openGL->switchMode(cnmode);
    ((TVWindow *)windowWgt)->openGL->SwitchToChan(myChanls->SwitchToChanl(0));
}

void TController::VolimeUp() const
{
    const int volgap = 3;
    int vol = volumeCtrl->upvolume(volgap);
    ((QAbstractSlider *)sliderVol)->setValue(vol);
    ((QAbstractButton *)volMuteBtn)->setChecked(0);
}

void TController::VolimeDown() const
{
    const int volgap = 3;
    int vol = volumeCtrl->downvolume(volgap);
    ((QAbstractSlider *)sliderVol)->setValue(vol);
}

void TController::VolimeMute() const
{
    bool stateMute = volumeCtrl->mute();
    ((QAbstractButton *)volMuteBtn)->setChecked(!stateMute);
}

void TController::VolimeSet(const int volPc) const
{
    volumeCtrl->setvolpc(volPc);
    ((QAbstractSlider *)sliderVol)->setValue(volPc);
    ((QAbstractButton *)volMuteBtn)->setChecked(0);
}

//void TController::VolimeUp() const

void TController::AddChannelsToMenu(QMenu *menu, QActionGroup *actGroup, QObject* parent) const
{
    const std::vector<std::string> &chnnms = myChanls->GetChanelsNames();
    int i=0;
    for (std::vector<std::string>::const_iterator itChn = chnnms.begin(); itChn != chnnms.end(); ++itChn) {
        QAction *tmpAct = new QAction(QString::number(i+1) +  QString(". ") +  QString::fromStdString(*itChn), parent);
        tmpAct->setCheckable(true);
        tmpAct->setData(i);
        if (myChanls->GetThisChanl().id == i)
            tmpAct->setChecked(true);
        actGroup->addAction(tmpAct);
        menu->addAction(tmpAct);
        ++i;
    }
}

void TController::ChangeScale(const TScreenFormat frmr, const int wndW, int &newH) const
{
    myChanls->SetThisChanlScreenFromat(frmr);
    ((TVWindow *)windowWgt)->openGL->switchScreenFrormat(frmr);
    ChangeHeightByScale(wndW, newH);
}

void TController::ChangeScale(const TScreenFormat frmr, int &newW, const int wndH) const
{
    myChanls->SetThisChanlScreenFromat(frmr);
    ((TVWindow *)windowWgt)->openGL->switchScreenFrormat(frmr);
    ChangeWidthByScale(newW, wndH);
}

void TController::ChangeHeightByScale(const int wndW, int &newH) const
{
    newH = wndW * 3 / 4;
    if (myChanls->GetThisChanl().iScreenFormat == frm16x9 || myChanls->GetThisChanl().iScreenFormat == frm16x9pan)
        newH = wndW * 9 / 16;
    else if (myChanls->GetThisChanl().iScreenFormat == frm16x10 || myChanls->GetThisChanl().iScreenFormat == frm16x10pan)
        newH = wndW * 10 / 16;
}

void TController::ChangeWidthByScale(int &newW, const int wndH) const
{
    newW = wndH * 4 / 3;
    if (myChanls->GetThisChanl().iScreenFormat == frm16x9 || myChanls->GetThisChanl().iScreenFormat == frm16x9pan)
        newW = wndH * 16 / 9;
    else if (myChanls->GetThisChanl().iScreenFormat == frm16x10 || myChanls->GetThisChanl().iScreenFormat == frm16x10pan)
        newW = wndH * 16 / 10;
}

void TController::ShowPropDialog() const
{
    //настройки
    if (!PropertiesDialog::isShowNow) {
        PropertiesDialog pw;
        pw.exec();
    }
}

void TController::SetVolSlider(void *volSlider, void *muteBtn)
{
    sliderVol = volSlider;
    volMuteBtn = muteBtn;
}

void TController::SwitchFullScrean()
{
    if (iFullScreen) {
        iFullScreen = 0;
        ((TVWindow *)windowWgt)->SwitchToWindow();
    } else {
        iFullScreen = 1;
        ((TVWindow *)windowWgt)->SwitchToFull();
    }
}

bool TController::isFullScrean() const
{
    return iFullScreen;
}
