#include <QtGui>
#include "audiocontrol.h"

TAudioControl::TAudioControl(const std::string &nameDev, const std::string &nameChan, const bool muteState) : mutestate(muteState)
{
    mixer = 0;
    snd_mixer_open((snd_mixer_t**)&mixer, 0);
    snd_mixer_attach(mixer, nameDev.c_str());
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&sid);

    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, nameChan.c_str());
    elem = snd_mixer_find_selem(mixer, sid);

    if (elem == NULL) {
        qDebug("[err] unable to find control %s, %i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
        return;
    }

    if (!snd_mixer_selem_has_playback_volume (elem))
        qDebug("[error] snd_mixer_selem_has_playback_volume");

    iMinVol = 0;
    iMaxVol = 0;
    snd_mixer_selem_get_playback_volume_range(elem, &iMinVol, &iMaxVol);
    int m_state;
    snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &m_state);
    if (m_state == 0)
        mutestate = 0;
    //qDebug("vol=%d-%d", iMinVol, iMaxVol);
}

TAudioControl::~TAudioControl()
{
    snd_mixer_close(mixer);
}

int TAudioControl::upvolume(const int gappc)
{
 int volpc = 0;
 if (elem) {
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &iVolChn);

    volpc = (int)((double) iVolChn * 100.0 / (iMaxVol-iMinVol) + 0.5) + gappc;
    if (volpc > 100)
        volpc = 100;
    iVolChn = (long)((double) volpc * (iMaxVol-iMinVol)/ 100.0 + 0.5);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, iVolChn);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, iVolChn);
    if (mutestate == 0) {
        snd_mixer_selem_set_playback_switch_all(elem, 1);
        mutestate = 1;
    }
    //qDebug("vol=%d", iVolChn);
 }
 return volpc;
}

int TAudioControl::downvolume(const int gappc)
{
 int volpc = 0;
 if (elem) {
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &iVolChn);

    volpc = (int)((double) iVolChn * 100.0 / (iMaxVol-iMinVol) + 0.5) - gappc;
    if (volpc < 0)
        volpc = 0;
    iVolChn = (long)((double) volpc * (iMaxVol-iMinVol)/ 100.0 + 0.5);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, iVolChn);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, iVolChn);

    //qDebug("vol=%d", iVolChn);
 }
 return volpc;
}

bool TAudioControl::mute()
{   
 mutestate = !mutestate;
 if (elem) {
  snd_mixer_selem_set_playback_switch_all(elem, mutestate);
 }
 return mutestate;
}

int TAudioControl::setvolpc(int volpc)
{
 if (elem) {
    iVolChn = (long)((double) volpc * (iMaxVol-iMinVol)/ 100.0 + 0.5);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, iVolChn);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, iVolChn);

    //qDebug("vol=%d", iVolChn);
 }
 return volpc;
}
