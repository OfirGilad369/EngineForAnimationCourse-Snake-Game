//
// Created by Ofir Gilad on 07/02/2023.
//

#include "SoundManager.h"

#include "thread"
#include <assert.h>
#include <chrono>
#include <future>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>


void SoundManager::HandleMusic(string music_file) {
    if (playing) {
        int updated_player_index = playing_index;
        StopMusic();
        playing_index = updated_player_index;
    }

    if (music_player_enabled) {
        // Calling a script to play the music
        std::string command = python_version + " scripts/new_play_music.py \"" + music_file + "\"";
        pipe = _popen(command.c_str(), "w");
        if (!pipe) {
            std::cerr << "Error: Could not open pipe to Python process." << std::endl;
            return;
        }
        playing = true;

        // Set Volume
        string updated_volume = to_string(music_volume);
        std::fprintf(pipe, "%s\n", updated_volume.c_str());
        fflush(pipe);
    }
}

void SoundManager::HandleSound(string sound_file) {
    // Calling a script to play the sound
    const auto& PlaySound = [&](std::string sound_file) {
        string updated_volume = to_string(sound_volume * 100);
        std::string command = python_version + " scripts/new_play_sound.py " + sound_file + " " + updated_volume + "";
        system(command.c_str());
    };

    if (sound_player_enabled) {
        std::thread t2(PlaySound, sound_file);
        t2.detach();
    }
}

void SoundManager::UpdateMusicPlayerVolume() {
    if (playing) {
        string updated_volume = to_string(music_volume);
        std::fprintf(pipe, "%s\n", updated_volume.c_str());
        fflush(pipe);
    }
}

void SoundManager::StopMusic() {
    if (playing) {
        playing_index = -1;

        // Kill All Media Players
        playing = false;
        std::string kill_command = "taskkill /f /im " + python_version + ".exe";
        system(kill_command.c_str());
    }
}

SoundManager::~SoundManager() {
    StopMusic();
}