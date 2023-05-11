//
// Created by Ofir Gilad on 21/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTSVISITOR_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTSVISITOR_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_GAMEOBJECTSVISITOR_H

#pragma once
#include "../tutorial/FinalProject/BasicScene.h"
#include "Visitor.h"
#include "Model.h"
#include <vector>

#include <igl/per_vertex_normals.h>
#include <iostream>
#include <random>
#include <utility>


namespace cg3d 
{
    class GameObjectsVisitor : public Visitor
	{
    public:
        void Run(Scene* scene, Camera* camera) override;
        void Visit(Scene* scene) override;
		
    private:
        GameManager* game_manager;
    };
}


