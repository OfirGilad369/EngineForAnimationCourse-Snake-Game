//
// Created by Ofir Gilad on 21/02/2023.
//

#include "GameObjectsVisitor.h"
#include "Utility.h"

void GameObjectsVisitor::Run(cg3d::Scene* _scene, cg3d::Camera* camera) 
{
    Visitor::Run(_scene, camera);
}

void GameObjectsVisitor::Visit(Scene* _scene)
{
    if (_scene->GetAnimate()) {
        game_manager = ((BasicScene*)_scene)->game_manager;

        // Handle Active Bonus
        string active_bonus = game_manager->stats->active_bonus;

        if (game_manager->stats->active_bonus != "None") {
            int bonuses_duration = game_manager->stats->bonuses_duration;
            int current_active_bonus_time = game_manager->stats->active_bonus_timer.GetElapsedTime();

            if (current_active_bonus_time > bonuses_duration) {
                game_manager->stats->active_bonus_timer.ResetTimer();
                game_manager->stats->active_bonus = "None";
            }
        }

        // Handle Freeze Time
        if (active_bonus == "Freeze Time") {
            game_manager->stage_timer.StopTimer();
        }
        else {
            game_manager->stage_timer.StartTimer();
        }

        // Handle Alive Game Objects
        for (int i = 0; i < int(game_manager->alive_objects.size()); i++) {
            GameObject* current_game_object = game_manager->alive_objects[i];
            float object_alive_time = current_game_object->alive_timer.GetElapsedTime();

            if (object_alive_time > 20.f * game_manager->stats->selected_stage) {
                // Move to Dead Objects
                game_manager->alive_objects.erase(game_manager->alive_objects.begin() + i);
                game_manager->dead_objects.push_back(current_game_object);

                // Handle Object Event
                current_game_object->SetDead();
            }
            else{
                current_game_object->MoveObject();
            }
        }

        // Handle Dead Game Objects
        for (int i = 0; i < int(game_manager->dead_objects.size()); i++) {
            GameObject* current_game_object = game_manager->dead_objects[i];
            float object_dead_time = current_game_object->dead_timer.GetElapsedTime();

            // Handle Exit Object
            if (current_game_object->model->name == "ExitObject") {
                if (game_manager->stats->current_score >= game_manager->stats->objective_score) {
                    // Move to Alive Objects
                    game_manager->dead_objects.erase(game_manager->dead_objects.begin() + i);
                    game_manager->alive_objects.push_back(current_game_object);

                    // Handle Object Event
                    current_game_object->SetAlive();
                    current_game_object->MoveObject();
                    cout << "Exit Appeared" << endl;

                    return;
                }
            }
            else if (object_dead_time > 5.f * game_manager->stats->selected_stage) {
                // Move to Alive Objects
                game_manager->dead_objects.erase(game_manager->dead_objects.begin() + i);
                game_manager->alive_objects.push_back(current_game_object);

                // Handle Object Event
                current_game_object->SetAlive();

                return;
            }
        }
    }
}
