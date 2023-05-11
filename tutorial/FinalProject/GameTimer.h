//
// Created by Ofir Gilad on 20/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMETIMER_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMETIMER_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMETIMER_H

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace std::chrono;

class GameTimer
{
public:
    GameTimer() {};
	
    void StartTimer();
    void StopTimer();
    void ResetTimer();
	
    string SecondsToGameTime(float seconds);
    int GameTimeToSecond(const string& timeString);

    double GetElapsedTime();
    void SetElapsedTime(float elapsed_time);
	
private:
    time_point<system_clock> start_time;
    bool running_status = false;
    double elapsed_time = 0;
};


