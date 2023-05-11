//
// Created by Ofir Gilad on 13/02/2023.
//

#include "GameManager.h"

void GameManager::InitGameManager(std::shared_ptr<Movable> _root, std::vector<std::shared_ptr<Camera>> _camera_list)
{
	root = _root;
	camera_list = _camera_list;

    // Init Collision Boxes
    InitCollisionBoxes();

    // Init Snake
    snake = Snake(root, camera_list);
    snake.InitSnake(number_of_bones);

    // Init Sound Manager
    sound_manager = new SoundManager();

    // Init Stats
    stats = new Stats();
    stats->InitStats();
    stats->snake_head = snake.GetBones()[0];

    // Init Leaderboard
    leaderboard = new Leaderboard();
    leaderboard->InitLeaderboard();

    // Init Objects Builder
    objects_builder = ObjectsBuilder();
    objects_builder.InitObjectsBuilder(stats, sound_manager);

    // Build all required objects
    InitCustomObjects();
    BuildGameObjects();
    BuildExit();

    // Init Game Timer
    InitTimers();
}

void GameManager::NewGame(string name) {
    game_timer.ResetTimer();

    stats->ResetStats(name);
    leaderboard->ResetLeaderboard();

    game_timer.StartTimer();
}

void GameManager::SaveGame() {
    float time_played = game_timer.GetElapsedTime();
    stats->time_played = game_timer.SecondsToGameTime(time_played);

    stats->SaveStats();
    leaderboard->SaveLeaderboard();
}

void GameManager::LoadStage(int stage_number, bool new_stage)
{
    UnloadStage();

    // Set Select Stage
    stats->selected_stage = stage_number;
    InitStageParameters(new_stage);
    InitBackground();
    InitAxis();
    LoadGameObjects();
    snake.ShowSnake();
    stage_timer.StartTimer();
}

void GameManager::UnloadStage() {
    stats->selected_stage = 0;

    if (background_loaded) {
        root->RemoveChild(background);
        background_loaded = false;
    }
    if (axis_loaded) {
        root->RemoveChild(axis);
        axis_loaded = false;
    }

    snake.HideSnake();
    snake.ResetSnakePosition();

    for (int i = 0; i < alive_objects.size(); i++) {
        alive_objects[i]->SetDead();
    }

    alive_objects.clear();
    dead_objects.clear();

    stage_timer.ResetTimer();

    stats->active_bonus = "None";
    stats->active_bonus_timer.ResetTimer();
}

void GameManager::PauseStageHandle(bool pause_status) {
    if (pause_status) {
        stage_timer.StopTimer();

        for (int i = 0; i < alive_objects.size(); i++) {
            alive_objects[i]->alive_timer.StopTimer();
        }

        for (int i = 0; i < dead_objects.size(); i++) {
            dead_objects[i]->dead_timer.StopTimer();
        }

        if (stats->active_bonus != "None") {
            stats->active_bonus_timer.StopTimer();
        }
    }
    else {
        stage_timer.StartTimer();

        for (int i = 0; i < alive_objects.size(); i++) {
            alive_objects[i]->alive_timer.StartTimer();
        }

        for (int i = 0; i < dead_objects.size(); i++) {
            dead_objects[i]->dead_timer.StartTimer();
        }

        if (stats->active_bonus != "None") {
            stats->active_bonus_timer.StartTimer();
        }
    }
}

void GameManager::ResetTimers() {
    stage_timer.ResetTimer();
    stage_timer.StartTimer();

    for (int i = 0; i < alive_objects.size(); i++) {
        alive_objects[i]->alive_timer.ResetTimer();
        alive_objects[i]->alive_timer.StartTimer();
    }

    for (int i = 0; i < dead_objects.size(); i++) {
        alive_objects[i]->alive_timer.ResetTimer();
        alive_objects[i]->alive_timer.StartTimer();
    }
}

void GameManager::InitCollisionBoxes() {
    auto program = std::make_shared<Program>("shaders/basicShader");
    auto material{ std::make_shared<Material>("material", program) }; // empty material
    auto cubeMesh1{ IglLoader::MeshFromFiles("cube1", "data/cube.off") };
    auto cubeMesh2{ IglLoader::MeshFromFiles("cube2", "data/cube.off") };
    cube1 = Model::Create("cube1", cubeMesh1, material);
    cube2 = Model::Create("cube2", cubeMesh1, material);
    cube1->showFaces = false;
    cube2->showFaces = false;
    cube1->showWireframe = true;
    cube2->showWireframe = true;
}

void GameManager::InitStageParameters(bool new_stage) {
    if (new_stage) {
        stats->current_score = 0;
    }

    stats->last_time_damage = 0;
    stats->current_movement_speed = stats->min_movement_speed;
    stats->current_health = stats->max_health;
    stats->objective_score = stats->current_score + 200 * stats->selected_stage;
}

void GameManager::InitBackground() {
    int stage_number = stats->selected_stage;

    auto daylight{ std::make_shared<Material>("daylight", "shaders/cubemapShader") };
    
    if (stats->selected_stage == 1) {
        daylight->AddTexture(0, "cubemaps/Stage1/Stage1 Box_", 3);
    }
    else if (stats->selected_stage == 2) {
        daylight->AddTexture(0, "cubemaps/Stage2/Stage2 Box_", 3);
    }
    else if (stats->selected_stage == 3) {
        daylight->AddTexture(0, "cubemaps/Stage3/Stage3 Box_", 3);
    }

    background = Model::Create("background", Mesh::Cube(), daylight);
    root->AddChild(background);
    background->Scale(2 * stats->base_length * stage_number, Movable::Axis::XYZ);
    background->SetPickable(false);
    background->SetStatic();

    background_loaded = true;
}

void GameManager::InitAxis() {
    int stage_number = stats->selected_stage;
    stats->stage_size = stats->base_length * stage_number;

    auto program = std::make_shared<Program>("shaders/basicShader");
    auto material{ std::make_shared<Material>("material", program) }; // empty material

    Eigen::MatrixXd vertices(6, 3);
    vertices << -1, 0, 0, 1, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, -1, 0, 0, 1;
    Eigen::MatrixXi faces(3, 2);
    faces << 0, 1, 2, 3, 4, 5;
    Eigen::MatrixXd vertexNormals = Eigen::MatrixXd::Ones(6, 3);
    Eigen::MatrixXd textureCoords = Eigen::MatrixXd::Ones(6, 2);
    std::shared_ptr<Mesh> coordsys = std::make_shared<Mesh>("coordsys", vertices, faces, vertexNormals, textureCoords);
    axis = Model::Create("axis", coordsys, material);
    axis->mode = 1;
    axis->Scale(stats->stage_size, Movable::Axis::XYZ);
    root->AddChild(axis);
    axis->Translate(Eigen::Vector3f(0, 0, 0));

    axis_loaded = true;
}

void GameManager::LoadGameObjects() {
    int selected_stage = stats->selected_stage;
    int number_of_objects_to_load = (max_number_of_objects / 3) * selected_stage;

    for (int i = 0; i < int(all_objects.size()); i++) {
        for (int j = 0; j < number_of_objects_to_load; j++) {
            all_objects[i][j]->SetAlive();
            alive_objects.push_back(all_objects[i][j]);
        }
    }

    exit_object->SetDead();
    dead_objects.push_back(exit_object);
}

void GameManager::InitCustomObjects() {
    // health
    auto program1 = std::make_shared<Program>("shaders/phongShader");
    auto material1 = std::make_shared<Material>("material", program1);
    material1->program->name = "health";
    health_model = ObjLoader::ModelFromObj("health", "objects/health.obj", material1);

    health_model->Scale(0.3f, Movable::Axis::XYZ);
    health_model->Translate(-1.5f, Movable::Axis::Z);

    // Score
    auto program2 = std::make_shared<Program>("shaders/phongShader");
    auto material2 = std::make_shared<Material>("material", program2);
    material2->program->name = "score";
    score_model = ObjLoader::ModelFromObj("score", "objects/score.obj", material2);

    score_model->Scale(0.17f, Movable::Axis::XYZ);
    score_model->Translate(-1.7f, Movable::Axis::Z);

    // Gold
    auto program3 = std::make_shared<Program>("shaders/phongShader");
    auto material3 = std::make_shared<Material>("material", program3);
    material3->program->name = "gold";
    gold_model = ObjLoader::ModelFromObj("gold", "objects/gold.obj", material3);

    gold_model->Scale(0.17f, Movable::Axis::XYZ);
    gold_model->Translate(-0.6f, Movable::Axis::Z);

    // Bonus
    auto program4 = std::make_shared<Program>("shaders/phongShader");
    auto material4 = std::make_shared<Material>("material", program4);
    material4->program->name = "bonus";
    bonus_model = ObjLoader::ModelFromObj("bonus", "objects/bonus.obj", material4);

    bonus_model->Scale(0.17f, Movable::Axis::XYZ);
    bonus_model->Translate(-1.7f, Movable::Axis::Z);

    // Obstacle
    auto program5 = std::make_shared<Program>("shaders/phongShader");
    auto material5 = std::make_shared<Material>("material", program5);
    material5->program->name = "obstacle";
    obstacle_model = ObjLoader::ModelFromObj("obstacle", "objects/obstacle.obj", material5);

    obstacle_model->Scale(0.68f, Movable::Axis::XYZ);
    obstacle_model->Translate(-10.0f, Movable::Axis::Z);

    // Exit
    auto program6 = std::make_shared<Program>("shaders/phongShader");
    auto material6 = std::make_shared<Material>("material", program6);
    material6->program->name = "exit";
    exit_model = ObjLoader::ModelFromObj("obstacle", "objects/exit.obj", material6);

    exit_model->Scale(0.15f, Movable::Axis::XYZ);
}

void GameManager::BuildGameObjects() {
    BuildHealthObjects();
    BuildScoreObjects();
    BuildGoldObjects();
    BuildBonusObjects();
    BuildObstacleObjects();
}

void GameManager::BuildHealthObjects() {
    std::vector<GameObject*> objects_list;

    for (int i = 0; i < max_number_of_objects; i++) {
        // Init meshes
        temp_object1 = Model::Create("health", health_model->GetMesh(), health_model->material);
        temp_object1->SetTransform(health_model->GetTransform());
        temp_object2 = Model::Create("HealthObject", Mesh::Cube(), health_model->material);
        //root->AddChild(temp_object2);

        // Adding to Collition Box
        temp_object2->AddChild(temp_object1);
        
        // Fix Scaling
        temp_object2->Scale(Eigen::Vector3f(3.4f, 3.4f, 3.4f));
        temp_object2->isHidden = true;
        //temp_object2->showFaces = false;

        // Adding to Stage objects list;
        objects_list.push_back(objects_builder.BuildGameObject(temp_object2, root));
    }

    all_objects.push_back(objects_list);
}

void GameManager::BuildScoreObjects() {
    std::vector<GameObject*> objects_list;

    for (int i = 0; i < max_number_of_objects; i++) {
        // Init meshes
        temp_object1 = Model::Create("score", score_model->GetMesh(), score_model->material);
        temp_object1->SetTransform(score_model->GetTransform());
        temp_object2 = Model::Create("ScoreObject", Mesh::Cube(), score_model->material);
        //root->AddChild(temp_object2);

        // Adding to Collition Box
        temp_object2->AddChild(temp_object1);

        // Fix Scaling
        temp_object2->Scale(Eigen::Vector3f(3.4f, 3.4f, 3.4f));
        temp_object2->isHidden = true;
        //temp_object2->showFaces = false;

        // Adding to Stage objects list
        objects_list.push_back(objects_builder.BuildGameObject(temp_object2, root));
    }

    all_objects.push_back(objects_list);
}

void GameManager::BuildGoldObjects() {
    std::vector<GameObject*> objects_list;

    for (int i = 0; i < max_number_of_objects; i++) {
        // Init meshes
        temp_object1 = Model::Create("gold", gold_model->GetMesh(), gold_model->material);
        temp_object1->SetTransform(gold_model->GetTransform());
        temp_object2 = Model::Create("GoldObject", Mesh::Cube(), gold_model->material);
        //root->AddChild(temp_object2);

        // Adding to Collition Box
        temp_object2->AddChild(temp_object1);

        // Fix Scaling
        temp_object2->Scale(Eigen::Vector3f(3.4f, 3.4f, 3.4f));
        temp_object2->isHidden = true;
        //temp_object2->showFaces = false;

        // Adding to Stage objects list
        objects_list.push_back(objects_builder.BuildGameObject(temp_object2, root));
    }

    all_objects.push_back(objects_list);
}

void GameManager::BuildBonusObjects() {
    std::vector<GameObject*> objects_list;

    for (int i = 0; i < max_number_of_objects; i++) {
        // Init meshes
        temp_object1 = Model::Create("bonus", bonus_model->GetMesh(), bonus_model->material);
        temp_object1->SetTransform(bonus_model->GetTransform());
        temp_object2 = Model::Create("BonusObject", Mesh::Cube(), bonus_model->material);
        //root->AddChild(temp_object2);

        // Adding to Collition Box
        temp_object2->AddChild(temp_object1);

        // Fix Scaling
        temp_object2->Scale(Eigen::Vector3f(3.4f, 3.4f, 3.4f));
        temp_object2->isHidden = true;
        //temp_object2->showFaces = false;

        // Adding to Stage objects list
        objects_list.push_back(objects_builder.BuildGameObject(temp_object2, root));
    }

    all_objects.push_back(objects_list);
}

void GameManager::BuildObstacleObjects() {
    std::vector<GameObject*> objects_list;

    for (int i = 0; i < max_number_of_objects; i++) {
        // Init meshes
        temp_object1 = Model::Create("obstacle", obstacle_model->GetMesh(), obstacle_model->material);
        temp_object1->SetTransform(obstacle_model->GetTransform());
        temp_object2 = Model::Create("ObstacleObject", Mesh::Cube(), obstacle_model->material);
        //root->AddChild(temp_object2);

        // Adding to Collition Box
        temp_object2->AddChild(temp_object1);

        // Fix Scaling
        temp_object2->Scale(Eigen::Vector3f(15.f, 15.f, 15.f));
        //temp_object2->Scale(Eigen::Vector3f(20.f, 20.f, 20.f));
        temp_object2->isHidden = true;
        //temp_object2->showFaces = false;


        // Adding to Stage objects list
        objects_list.push_back(objects_builder.BuildGameObject(temp_object2, root));
    }

    all_objects.push_back(objects_list);
}

void GameManager::BuildExit() {
    // Init meshes
    temp_object1 = Model::Create("exit", exit_model->GetMesh(), exit_model->material);
    temp_object1->SetTransform(exit_model->GetTransform());
    exit = Model::Create("ExitObject", Mesh::Cube(), exit_model->material);
    //root->AddChild(exit);

    // Adding to Collition Box
    exit->AddChild(temp_object1);
    //exit->Translate(-60, Movable::Axis::Z);

    // Fix Scaling
    exit->Scale(Eigen::Vector3f(3.f, 3.f, 1.f));
    exit->isHidden = true;
    //temp_object2->showFaces = false;

    exit_object = objects_builder.BuildGameObject(exit, root);
}

void GameManager::InitTimers() {
    game_timer = GameTimer();
    string game_time = stats->time_played;
    float elapsed_time = float(game_timer.GameTimeToSecond(game_time));
    game_timer.SetElapsedTime(elapsed_time);
    game_timer.StartTimer();

    stage_timer = GameTimer();
}
