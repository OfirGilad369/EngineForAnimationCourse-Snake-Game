//
// Created by Ofir Gilad on 10/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_COLLISIONDETECTIONVISITOR_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_COLLISIONDETECTIONVISITOR_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_COLLISIONDETECTIONVISITOR_H

#include "Visitor.h"
#include "Model.h"
#include <vector>
#include "../tutorial/FinalProject/BasicScene.h"
#include "../tutorial/FinalProject/GameLogics.h"

#include <AABB.h>
#include "igl/read_triangle_mesh.cpp"
#include "igl/edge_flaps.h"
#include "igl/per_vertex_normals.h"


namespace cg3d
{
    class CollisionDetectionVisitor :public Visitor
    {
    public:
        void Run(Scene *scene, Camera *camera) override;
        void Visit(Scene* scene) override;

    private:
        void TimerDamage();
        bool CheckSelfCollision();
        bool CheckBackgoroundCollision();

        GameManager* game_manager;
        GameLogics collision_logic;

        Eigen::Vector3f backgound_cube_space;
    };
}
