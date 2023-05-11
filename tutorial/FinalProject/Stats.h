//
// Created by Ofir Gilad on 13/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_STATS_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_STATS_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_STATS_H

#include <string>
#include "json.hpp"
#include <fstream>
#include <iostream>
#include "Scene.h"

#include "GameTimer.h"

using namespace std;
using namespace nlohmann;

class Stats
{
public:
    Stats() {};

    void InitStats();
    void ResetStats(string name);
    void SaveStats();

    bool save_data_available = false;

    // User data
    string user_name = "Guest";
    int gold = 0;


    // Snake stats Default Values
    int max_health = 100;
    int score_multiplier = 1;
    int gold_multiplier = 1;
    int bonuses_duration = 10;
    int max_movement_speed = 3;


    // Statistics default values
    int total_health_points_healed = 0;
    int total_health_points_lost = 0;
    int total_score_points_earned = 0;
    int total_gold_earned = 0;
    int total_gold_spent = 0;
    int total_bonuses_collected = 0;
    int total_deaths = 0;
    string time_played = "00:00:00";

    // Stage stats
    bool stage_completed = false;
    int selected_stage = 0;
    float base_length = 60.f;
    float stage_size = 0.f;
    int current_health = 100;
    int last_time_damage = 0;
    int current_score = 0;
    int objective_score = 0;
    int current_movement_speed = 1;
    int min_movement_speed = 1; // Always
    int leaderboard_position = -1;

    // Active bonus Parameters
    GameTimer active_bonus_timer;
    std::shared_ptr<cg3d::Model> snake_head;
    string active_bonus = "None";

private:
    json save_data;
};
