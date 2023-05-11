//
// Created by Ofir Gilad on 05/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_SNAKE_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_SNAKE_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_SNAKE_H

#pragma once
#include "Scene.h"
#include "IglMeshLoader.h"
#include "ObjLoader.h"

#include "igl/per_vertex_normals.h"
#include <igl/dqs.h>

#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <vector>
#include <algorithm>
#include <iostream>

#include <memory>
#include <utility>

using namespace cg3d;
using namespace std;

class Snake
{
public:
    Snake() {};
    Snake(std::shared_ptr<Movable> root, vector<std::shared_ptr<Camera>> camera_list);
    void InitSnake(int num_of_bones);

    void ShowSnake();
    void HideSnake();
    void ResetSnakePosition();

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();
    void RollLeft();
    void RollRight();

    void UpdateCameraView();
    void ResetCameraView();

    void SkinningInit();
    void Skinning();

    std::vector<std::shared_ptr<cg3d::Model>> GetBones() { return snake_bones; }
    Eigen::Vector3f GetBonePosition(int bone_id, int position);

    // Skinning status
    bool skinning_enabled = false;

private:
    void CalculateWeight();

    
    vector<std::shared_ptr<Camera>> camera_list;
    std::shared_ptr<Movable> root;
    std::vector<std::shared_ptr<Model>> snake_bones;
    std::shared_ptr <Model> snake_head, snake_tail, snake_body;
    float bone_size = 1.6f;
    int number_of_bones = 0;
    int first_index = 0;
    int last_index = 0;
    int view_state = 0;
    float snake_length = 0;


    // Skinning Part
    typedef
        std::vector<Eigen::Quaterniond, Eigen::aligned_allocator<Eigen::Quaterniond>>
        RotationList;

    // W - weights matrix
    // BE - Edges between joints
    // C - joints positions
    // P - parents
    // M - weights per vertex per joint matrix
    // U - new vertices position after skinning
    Eigen::MatrixXd C, OV, V, W, M, U;
    Eigen::MatrixXi BE;
    Eigen::VectorXi P;


    // Propagate relative rotations via FK to retrieve absolute transformations
    // vQ - rotations of joints
    // vT - translation of joints
    RotationList vQ;
    vector<Eigen::Vector3d> vT;
};