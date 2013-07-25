#ifndef AUDIOCONTROL_H
#define AUDIOCONTROL_H

#include <string>
#include <alsa/asoundlib.h>

class TAudioControl
{
    snd_mixer_t* mixer;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;

    long iMinVol;
    long iMaxVol;
    long iVolChn;

    bool mutestate;

public:
    TAudioControl(const std::string &nameDev="hw:0", const std::string &nameChan="CD", const bool muteState=0);
    ~TAudioControl();
    int downvolume(const int gappc);
    int upvolume(const int gappc);
    bool mute();
    int setvolpc(const int volpc);

};

#endif // AUDIOCONTROL_H
