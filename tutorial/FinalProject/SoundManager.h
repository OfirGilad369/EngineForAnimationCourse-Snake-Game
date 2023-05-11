//
// Created by Ofir Gilad on 07/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_SOUNDMANAGER_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_SOUNDMANAGER_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_SOUNDMANAGER_H

#include <string>
#include <thread>
#include <iostream>

using namespace std;

class SoundManager
{
public:
    SoundManager() {};

    void HandleMusic(string music_file);
    void HandleSound(string sound_file);
    
    void UpdateMusicPlayerVolume();

    void StopMusic();
    ~SoundManager();

    // Global Indexed
    int playing_index = -1;
    int stage_index = 0;

    bool music_player_enabled = true;
    float music_volume = 0.5f;

    bool sound_player_enabled = true;
    float sound_volume = 0.7f;

private:
    FILE* pipe;

    // Make sure to set the correct python version
    string python_version = "python3.10";

    bool playing = false;
};