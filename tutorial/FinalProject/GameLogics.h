//
// Created by Ofir Gilad on 06/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_MENU_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_MENU_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_MENU_H

#pragma once
#include "Scene.h"
#include "igl/AABB.h"

#include <memory>
#include <utility>
#include <random>

using namespace cg3d;
using namespace std;

class GameLogics
{
public:
    GameLogics() {};

    // Simplification
    void InitSimplification(vector<std::shared_ptr<cg3d::Model>> models);
    void level_up(int object_index);
    void level_down(int object_index);
    void level_reset(int object_index);
    void set_mesh_data(int object_index);
    void new_reset();
    void init_data(int object_index);
    void Q_matrix_calculation(int object_index);
    void edges_cost_calculation(int edge, int object_index);
    void new_simplification(int object_index);
    bool new_collapse_edge(int object_index);


    // Collision Detection
    void InitCollisionDetection(std::shared_ptr<cg3d::Model>model1, std::shared_ptr<cg3d::Model> model2, std::shared_ptr<cg3d::Model> cub1, std::shared_ptr<cg3d::Model> cube2);
    void AlignedBoxTransformer(Eigen::AlignedBox<double, 3>& aligned_box, std::shared_ptr<cg3d::Model> cube_model);
    bool CollisionCheck(igl::AABB<Eigen::MatrixXd, 3>* aligned_box1, igl::AABB<Eigen::MatrixXd, 3>* aligned_box2, int level);
    bool BoxesIntersectionCheck(Eigen::AlignedBox<double, 3>& aligned_box1, Eigen::AlignedBox<double, 3>& aligned_box2);


    // Inverse Kinematics
    void InitInverseKinematics(vector<std::shared_ptr<cg3d::Model>> arms, std::shared_ptr<cg3d::Model> destination, bool animate);
    Eigen::Vector3f GetDestinationPosition();
    Eigen::Vector3f GetLinkTipPosition(int link_id);
    Eigen::Vector3f GetLinkSourcePosition(int link_id);
    std::vector<Eigen::Matrix3f> GetEulerAnglesMatrices(Eigen::Matrix3f R);

    void IKCyclicCoordinateDecentMethod();
    void IKFabrikMethod();
    void IKSolverHelper(int link_id, Eigen::Vector3f D);


    // Bezier Curve
    void InitBezierCurve(std::shared_ptr<cg3d::Model> model, float stage_size);
    void GenerateBezierCurve();
    void MoveOnCurve();
    Eigen::Vector3f GenerateRandomPosition(int point_zone);

    std::shared_ptr<Model> GetBezierCurveModel() { return bezier_curve; }
    

private:
    // Simplification
    vector<std::shared_ptr<cg3d::Model>> models;

    int max_support;
    std::vector<int> num_collapsed;
    std::vector<int> indices;
    std::vector<int> current_available_collapses;

    std::vector<Eigen::VectorXi> EMAP;
    std::vector<Eigen::MatrixXi> F, E, EF, EI;
    std::vector<Eigen::VectorXi> EQ;
    std::vector<Eigen::MatrixXd> V, C;

    std::vector<Eigen::MatrixXi> OF;
    std::vector<Eigen::MatrixXd> OV;
    Eigen::MatrixXd VN, FN, T;

    typedef std::set<std::pair<double, int>> PriorityQueue;
    std::vector<PriorityQueue> new_Q; // priority queue - cost for every edge
    std::vector<std::vector<PriorityQueue::iterator>> Q_iter;
    std::vector<std::vector<Eigen::Matrix4d>> Q_matrix; // list of Q matrix for each vertical


    // Collision Detection
    std::shared_ptr<cg3d::Model> model1, model2, cube1, cube2;


    // Inverse Kinematics
    vector<std::shared_ptr<cg3d::Model>> arms;
    std::shared_ptr<cg3d::Model> destination;
    bool animate;

    int IK_mode = 0;
    bool animate_CCD = false;
    bool animate_Fabrik = false;
    float delta = 0.05f;
    float angle_divider = 50.f;

    int first_link_id = 0;
    int last_link_id = 15;
    int num_of_links = 16;
    float link_length = 1.6f;


    // Bezier Curve
    std::shared_ptr<Model> model, bezier_curve;
    float stage_size = 0.f;
    float t = 0.f;
    bool move_forward = true;
    int total_curve_points = 20;
    Eigen::Matrix<float, 4, 3> MG;

    float bezier_step = 0.001f;
};