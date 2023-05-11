//
// Created by Ofir Gilad on 13/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTS_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTS_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTS_H

#include <string>
#include "Scene.h"
#include "SoundManager.h"
#include "Stats.h" // Includes: GameTimer
#include "GameLogics.h"

using namespace cg3d;

class GameObject
{
public:
    virtual void InitObject(Stats* stats, SoundManager* sound_manager, std::shared_ptr<Model> model, std::shared_ptr<Movable> root);
    virtual void CollisionWithObject() = 0;
    virtual void MoveObject() = 0;
    virtual void SetAlive() = 0;
    virtual void SetDead() = 0;
    virtual Eigen::Vector3f GenerateRandomPosition();
    virtual ~GameObject() {}

    Stats* stats;
    SoundManager* sound_manager;
    std::shared_ptr<Movable> root;
    std::shared_ptr<Model> model, bezier_curve;
    GameTimer alive_timer, dead_timer;
    GameLogics bezier_logic;
};

class HealthObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;

private:
    int health_value, current_health, max_health;
};

class ScoreObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;

private:
    int score_value, current_score, score_multiplier;
};

class GoldObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;

private:
    int gold_value, gold, gold_multiplier;
};

class BonusObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;

private:
    int bonus_value, bonus_duration;
};

class ObstacleObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;

private:
    int damage_value, current_health;
};

class ExitObject : public GameObject
{
public:
    void CollisionWithObject() override;
    void MoveObject() override;
    void SetAlive() override;
    void SetDead() override;
};


class ObjectsBuilder
{
public:
    ObjectsBuilder() {};
    void InitObjectsBuilder(Stats* stats, SoundManager* sound_manager);
    GameObject* BuildGameObject(std::shared_ptr<Model> model, std::shared_ptr<Movable> root);

    Stats* stats;
    SoundManager* sound_manager;
};
