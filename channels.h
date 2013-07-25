#ifndef CHANNELS_H
#define CHANNELS_H

#include <vector>
#include <string>

enum TChanType {S_VIDEO, COMPOSIT, TV, FM};
enum TScreenFormat {frm4x3, frm16x9, frm16x9pan, frm16x10, frm16x10pan};

struct TChanParams {
    int id;
    std::string sName;
    int iFreqHz;
    int iSkip;
    int iVideoMode;
    int iAudioMode;
    int iAudioChannel;
    int iVolume;
    int iBrightness;
    int iContrast;
    int iHue;
    int iSaturation;
    int iSharpness;
    int iVideoGain;
    std::string sTVGuide;
    int iTVGuideOffset;
    TScreenFormat iScreenFormat;
};

class TChannels
{
protected:
    std::vector<TChanParams> chanslist;

    std::vector<TChanParams>::iterator ThisChan; //! текуший канал
    std::vector<TChanParams>::iterator PrevChan; //! предыдущий канал

public:
    TChannels();
    ~TChannels();

    void ReloadChanlList(const TChanType ChanType);
    const TChanParams &GetThisChanl() const;
    const TChanParams &SwitchToChanl(int NumChan);
    const TChanParams &SwitchToNextChanl();
    const TChanParams &SwitchToBackChanl();
    const TChanParams &SwitchToPrevChanl();
    int GetChanlsCount() const;
    std::vector<std::string> GetChanelsNames() const;
    int SetThisChanlScreenFromat(const TScreenFormat ScreenFormat);
};

#endif // CHANNELS_H
