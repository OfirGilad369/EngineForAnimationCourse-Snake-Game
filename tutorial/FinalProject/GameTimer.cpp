//
// Created by Ofir Gilad on 20/02/2023.
//

#include "GameTimer.h"


void GameTimer::StartTimer() {
    if (!running_status) {
        running_status = true;
        start_time = system_clock::now();
    }
}

void GameTimer::StopTimer() {
    if (running_status) {
        running_status = false;
        elapsed_time += duration<double>(system_clock::now() - start_time).count();
    }
}

void GameTimer::ResetTimer() {
    running_status = false;
    elapsed_time = 0;
}

string GameTimer::SecondsToGameTime(float seconds)
{
    ostringstream os;

    int hours = int(seconds) / 3600;
    int minutes = (int(seconds) % 3600) / 60;
    int secs = int(seconds) % 60;

    os << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << secs;

    return os.str();
}

int GameTimer::GameTimeToSecond(const string& timeString) {
    stringstream ss(timeString);
    string segment;
    int seconds = 0, minutes = 0, hours = 0;

    // Parse the time string into hours, minutes, and seconds
    std::getline(ss, segment, ':');
    if (!segment.empty()) {
        hours = std::stoi(segment);
    }
    std::getline(ss, segment, ':');
    if (!segment.empty()) {
        minutes = std::stoi(segment);
    }
    std::getline(ss, segment, ':');
    if (!segment.empty()) {
        seconds = std::stoi(segment);
    }

    // Calculate the total number of seconds
    return hours * 3600 + minutes * 60 + seconds;
}

double GameTimer::GetElapsedTime() {
    if (running_status) {
        return elapsed_time + duration<double>(system_clock::now() - start_time).count();
    } else {
        return elapsed_time;
    }
}

void GameTimer::SetElapsedTime(float elapsed_time) {
    this->elapsed_time = elapsed_time;
}
