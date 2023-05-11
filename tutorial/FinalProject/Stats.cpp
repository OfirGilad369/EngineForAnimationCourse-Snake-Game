//
// Created by Ofir Gilad on 13/02/2023.
//

#include "Stats.h"


void Stats::InitStats() {
    std::ifstream file("jsons/savedata.json");

    if (file.good()) {
        std::cout << "Savedata exists! \nReading data" << std::endl;
        save_data_available = true;
        // Read savedata
        file >> save_data;

        // User data
        user_name = save_data["user_data"]["username"];
        gold = save_data["user_data"]["gold"];


        // Snake stats
        max_health = save_data["snake_stats"]["max_health"];
        score_multiplier = save_data["snake_stats"]["score_multiplier"];
        gold_multiplier = save_data["snake_stats"]["gold_multiplier"];
        bonuses_duration = save_data["snake_stats"]["bonuses_duration"];
        max_movement_speed = save_data["snake_stats"]["max_movement_speed"];


        // Statistics
        total_health_points_healed = save_data["statistics"]["total_health_points_healed"];
        total_health_points_lost = save_data["statistics"]["total_health_points_lost"];
        total_score_points_earned = save_data["statistics"]["total_score_points_earned"];
        total_gold_earned = save_data["statistics"]["total_gold_earned"];
        total_gold_spent = save_data["statistics"]["total_gold_spent"];
        total_bonuses_collected = save_data["statistics"]["total_bonuses_collected"];
        total_deaths = save_data["statistics"]["total_deaths"];
        time_played = save_data["statistics"]["time_played"];
    }
    else {
        std::cout << "Savedata does not exist!" << std::endl;
        save_data_available = false;
    }

    active_bonus_timer = GameTimer();
}

void Stats::ResetStats(string name) {
    if (save_data_available) {
        save_data = json();
    }

    // User data
    user_name = name;

    save_data["user_data"]["username"] = user_name;
    save_data["user_data"]["gold"] = gold;


    // Snake stats
    save_data["snake_stats"]["max_health"] = max_health;
    save_data["snake_stats"]["score_multiplier"] = score_multiplier;
    save_data["snake_stats"]["gold_multiplier"] = gold_multiplier;
    save_data["snake_stats"]["bonuses_duration"] = bonuses_duration;
    save_data["snake_stats"]["max_movement_speed"] = max_movement_speed;
    
    
    // Statistics
    save_data["statistics"]["total_health_points_healed"] = total_health_points_healed;
    save_data["statistics"]["total_health_points_lost"] = total_health_points_lost;
    save_data["statistics"]["total_score_points_earned"] = total_score_points_earned;
    save_data["statistics"]["total_gold_earned"] = total_gold_earned;
    save_data["statistics"]["total_gold_spent"] = total_gold_spent;
    save_data["statistics"]["total_bonuses_collected"] = total_bonuses_collected;
    save_data["statistics"]["total_deaths"] = total_deaths;
    save_data["statistics"]["time_played"] = time_played;


    std::ofstream outfile("jsons/savedata.json");
    outfile << save_data.dump(4) << std::endl;
}


void Stats::SaveStats() {
    // User data
    save_data["user_data"]["username"] = user_name;
    save_data["user_data"]["gold"] = gold;


    // Snake stats
    save_data["snake_stats"]["max_health"] = max_health;
    save_data["snake_stats"]["score_multiplier"] = score_multiplier;
    save_data["snake_stats"]["gold_multiplier"] = gold_multiplier;
    save_data["snake_stats"]["bonuses_duration"] = bonuses_duration;
    save_data["snake_stats"]["max_movement_speed"] = max_movement_speed;


    // Statistics
    save_data["statistics"]["total_health_points_healed"] = total_health_points_healed;
    save_data["statistics"]["total_health_points_lost"] = total_health_points_lost;
    save_data["statistics"]["total_score_points_earned"] = total_score_points_earned;
    save_data["statistics"]["total_gold_earned"] = total_gold_earned;
    save_data["statistics"]["total_gold_spent"] = total_gold_spent;
    save_data["statistics"]["total_bonuses_collected"] = total_bonuses_collected;
    save_data["statistics"]["total_deaths"] = total_deaths;
    save_data["statistics"]["time_played"] = time_played;

    std::ofstream outfile("jsons/savedata.json");
    outfile << save_data.dump(4) << std::endl;
}
