#ifndef TCONTROLLER_H
#define TCONTROLLER_H

#include "audiocontrol.h"
#include "channels.h"
#include "tvwindow.h"

class TController
{
    TAudioControl *volumeCtrl;
    TChannels *myChanls;
    void *windowWgt;
    void *sliderVol;
    void *volMuteBtn;
    bool iFullScreen;

    static TController *ct;

    TController();
    TController(const TController&);
    const TController &operator=(const TController&) const;

public:
    ~TController();

    static TController *get();
    static void Uninit();

    void SetWindowsWgt(void *windowWidget);

    void SwitchChanNum(const int chan) const;
    void SwitchChanNext() const;
    void SwitchChanPrev() const;
    void SwitchChanBack() const;

    void SwitchTunerMode(const TChanType cnmode) const;

    void SetVolSlider(void *volSlider, void *muteBtn);

    void VolimeUp() const;
    void VolimeDown() const;
    void VolimeMute() const;
    void VolimeSet(const int volPc) const;

    void AddChannelsToMenu(QMenu *menu, QActionGroup *actGroup, QObject* parent) const;
    void ChangeScale(const TScreenFormat frmr, const int wndW, int &newH) const;
    void ChangeScale(const TScreenFormat frmr, int &newW, const int wndH) const;
    void ChangeHeightByScale(const int wndW, int &newH) const;
    void ChangeWidthByScale(int &newW, const int wndH) const;

    void ShowPropDialog() const;

    void SwitchFullScrean();
    bool isFullScrean() const;

};

#endif // TCONTROLLER_H
