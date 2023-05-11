//
// Created by Ofir Gilad on 06/02/2023.
//

#include "GameLogics.h"
#include "igl/edge_flaps.h"
#include "igl/per_vertex_normals.h"
#include "igl/vertex_triangle_adjacency.h"
#include "igl/per_face_normals.h"
#include "igl/circulation.h"
#include "igl/collapse_edge.h"

using namespace cg3d;


// Simplification

void GameLogics::InitSimplification(vector<std::shared_ptr<cg3d::Model>> models) {
    this->models = models;
}

void GameLogics::level_up(int object_index)
{
    indices[object_index]--;
    if (indices[object_index] < 0)
    {
        indices[object_index] = max(0, current_available_collapses[object_index] - 1);
    }
    models[object_index]->meshIndex = indices[object_index];
}

void GameLogics::level_down(int object_index)
{
    indices[object_index]++;
    if (indices[object_index] >= current_available_collapses[object_index])
    {
        indices[object_index] = 0;
    }
    models[object_index]->meshIndex = indices[object_index];
}

void GameLogics::level_reset(int object_index)
{
    indices[object_index] = 0;
    models[object_index]->meshIndex = indices[object_index];
}

void GameLogics::set_mesh_data(int object_index)
{
    igl::per_vertex_normals(V[object_index], F[object_index], VN);
    T = Eigen::MatrixXd::Zero(V[object_index].rows(), 2);
    auto mesh = models[object_index]->GetMeshList();
    mesh[0]->data.push_back({ V[object_index], F[object_index], VN, T });
    models[object_index]->SetMeshList(mesh);
    models[object_index]->meshIndex = indices[object_index];
}

void GameLogics::new_reset()
{
    int size = models.size();
    E.resize(size);
    EMAP.resize(size);
    EF.resize(size);
    new_Q.resize(size);
    EI.resize(size);
    C.resize(size);
    Q_iter.resize(size);
    Q_matrix.resize(size);
    num_collapsed.resize(size);

    for (int object_index = 0; object_index < size; object_index++) {
        auto mesh = models[object_index]->GetMeshList();
        for (int i = 1; i < current_available_collapses[object_index]; i++)
        {
            mesh[0]->data.pop_back();
        }
        models[object_index]->SetMeshList(mesh);
        current_available_collapses[object_index] = 1;

        V[object_index] = OV[object_index];
        F[object_index] = OF[object_index];
        init_data(object_index);
        indices[object_index] = 0;
        models[object_index]->meshIndex = indices[object_index];
    }
}

void GameLogics::init_data(int object_index)
{
    igl::edge_flaps(F[object_index], E[object_index], EMAP[object_index], EF[object_index], EI[object_index]); // Init data_structures
    C[object_index].resize(E[object_index].rows(), V[object_index].cols());
    Q_iter[object_index].resize(E[object_index].rows()); // Number of edges 
    Q_matrix_calculation(object_index);
    new_Q[object_index].clear();
    num_collapsed[object_index] = 0;

    // Caculate egdes cost
    for (int i = 0; i < E[object_index].rows(); i++)
    {
        edges_cost_calculation(i, object_index);
    }
}

void GameLogics::Q_matrix_calculation(int object_index)
{
    std::vector<std::vector<int>> VF;  // Vertex to faces
    std::vector<std::vector<int>> VFi; // Not in use
    int n = V[object_index].rows();
    Q_matrix[object_index].resize(n);
    igl::vertex_triangle_adjacency(n, F[object_index], VF, VFi);
    igl::per_face_normals(V[object_index], F[object_index], FN);

    for (int i = 0; i < n; i++)
    {
        // Initialize 
        Q_matrix[object_index][i] = Eigen::Matrix4d::Zero();

        // Caculate vertex Q matrix 
        for (int j = 0; j < VF[i].size(); j++)
        {
            // Get face normal
            Eigen::Vector3d normal = FN.row(VF[i][j]).normalized();

            // The equation is: ax+by+cz+d=0
            double a = normal[0];
            double b = normal[1];
            double c = normal[2];
            double d = V[object_index].row(i) * normal;
            d *= -1;

            // Kp = pp^T (s.t. p in planes)
            Eigen::Matrix4d Kp;
            Kp.row(0) = Eigen::Vector4d(a * a, a * b, a * c, a * d);
            Kp.row(1) = Eigen::Vector4d(a * b, b * b, b * c, b * d);
            Kp.row(2) = Eigen::Vector4d(a * c, b * c, c * c, c * d);
            Kp.row(3) = Eigen::Vector4d(a * d, b * d, c * d, d * d);
            Q_matrix[object_index][i] += Kp;
        }
    }
}

void GameLogics::edges_cost_calculation(int edge, int object_index)
{
    // Vertexes of the edge
    int v1 = E[object_index](edge, 0);
    int v2 = E[object_index](edge, 1);
    Eigen::Matrix4d Q_edge = Q_matrix[object_index][v1] + Q_matrix[object_index][v2];

    // We will use this to find v' position
    Eigen::Matrix4d Q_position = Q_edge;
    Q_position.row(3) = Eigen::Vector4d(0, 0, 0, 1);
    Eigen::Vector4d v_position;
    double cost;
    bool isInversable;
    Q_position.computeInverseWithCheck(Q_position, isInversable);

    if (isInversable)
    {
        v_position = Q_position * (Eigen::Vector4d(0, 0, 0, 1));
        cost = v_position.transpose() * Q_edge * v_position;
    }
    else
    {
        // Find min error from v1, v2, (v1+v2)/2
        Eigen::Vector4d v1_position;
        v1_position << V[object_index].row(v1), 1;
        double cost1 = v1_position.transpose() * Q_edge * v1_position;

        Eigen::Vector4d v2_position;
        v2_position << V[object_index].row(v2), 1;
        double cost2 = v2_position.transpose() * Q_edge * v2_position;

        Eigen::Vector4d v1v2_position;
        v1v2_position << ((V[object_index].row(v1) + V[object_index].row(v2)) / 2), 1;
        double cost3 = v1v2_position.transpose() * Q_edge * v1v2_position;

        if (cost1 < cost2 && cost1 < cost3)
        {
            v_position = v1_position;
            cost = cost1;
        }
        else if (cost2 < cost1 && cost2 < cost3)
        {
            v_position = v2_position;
            cost = cost2;
        }
        else {
            v_position = v1v2_position;
            cost = cost3;
        }
    }
    Eigen::Vector3d new_position;
    new_position[0] = v_position[0];
    new_position[1] = v_position[1];
    new_position[2] = v_position[2];
    C[object_index].row(edge) = new_position;
    Q_iter[object_index][edge] = new_Q[object_index].insert(std::pair<double, int>(cost, edge)).first;
}

void GameLogics::new_simplification(int object_index)
{
    // If it isn't the last collapsed mesh, do nothing
    if (indices[object_index] != current_available_collapses[object_index] - 1)
    {
        return;
    }
    bool something_collapsed = false;

    // Collapse 10% of edges
    const int max_iter = std::ceil(0.1 * new_Q[object_index].size());
    for (int i = 0; i < max_iter; i++)
    {
        if (!new_collapse_edge(object_index))
        {
            break;
        }
        something_collapsed = true;
        num_collapsed[object_index]++;
    }
    if (something_collapsed)
    {
        current_available_collapses[object_index]++;
        indices[object_index]++;
        set_mesh_data(object_index);
    }
}

bool GameLogics::new_collapse_edge(int object_index)
{
    PriorityQueue& curr_Q = new_Q[object_index];
    std::vector<PriorityQueue::iterator>& curr_Q_iter = Q_iter[object_index];
    int e1, e2, f1, f2; // Will be used in the igl collapse_edge function
    if (curr_Q.empty())
    {
        // No edges to collapse
        return false;
    }
    std::pair<double, int> pair = *(curr_Q.begin());
    if (pair.first == std::numeric_limits<double>::infinity())
    {
        // Min cost edge is infinite cost
        return false;
    }
    curr_Q.erase(curr_Q.begin()); // Delete from the queue
    int e = pair.second; // The lowest cost edge in the queue

    // The 2 vertices of the edge
    int v1 = E[object_index].row(e)[0];
    int v2 = E[object_index].row(e)[1];
    curr_Q_iter[e] = curr_Q.end();

    // Get the list of faces around the end point the edge
    std::vector<int> N = igl::circulation(e, true, EMAP[object_index], EF[object_index], EI[object_index]);
    std::vector<int> Nd = igl::circulation(e, false, EMAP[object_index], EF[object_index], EI[object_index]);
    N.insert(N.begin(), Nd.begin(), Nd.end());

    // Collapse the edage
    bool is_collapsed = igl::collapse_edge(e, C[object_index].row(e), V[object_index], F[object_index], E[object_index], EMAP[object_index], EF[object_index], EI[object_index], e1, e2, f1, f2);
    if (is_collapsed)
    {
        // Erase the two, other collapsed edges
        curr_Q.erase(curr_Q_iter[e1]);
        curr_Q_iter[e1] = curr_Q.end();
        curr_Q.erase(curr_Q_iter[e2]);
        curr_Q_iter[e2] = curr_Q.end();

        // Update the Q matrix for the 2 veterixes we collapsed 
        Q_matrix[object_index][v1] = Q_matrix[object_index][v1] + Q_matrix[object_index][v2];
        Q_matrix[object_index][v2] = Q_matrix[object_index][v1] + Q_matrix[object_index][v2];
        Eigen::VectorXd new_position;

        // Update local neighbors
        // Loop over original face neighbors
        for (auto n : N)
        {
            if (F[object_index](n, 0) != IGL_COLLAPSE_EDGE_NULL ||
                F[object_index](n, 1) != IGL_COLLAPSE_EDGE_NULL ||
                F[object_index](n, 2) != IGL_COLLAPSE_EDGE_NULL)
            {
                for (int v = 0; v < 3; v++)
                {
                    // Get edge id
                    const int ei = EMAP[object_index](v* F[object_index].rows() + n);
                    // Erase old entry
                    curr_Q.erase(curr_Q_iter[ei]);
                    // Compute cost and potential placement and place in queue
                    edges_cost_calculation(ei, object_index);
                    new_position = C[object_index].row(ei);
                }
            }
        }
        //cout << "Edge: " << e
        //    << ", Cost: " << pair.first
        //    << ", New Position: (" << new_position[0] << "," << new_position[1] << "," << new_position[2] << ")"
        //    << std::endl;
    }
    else
    {
        // Reinsert with infinite weight (the provided cost function must **not**
        // have given this un-collapsable edge inf cost already)
        pair.first = std::numeric_limits<double>::infinity();
        curr_Q_iter[e] = curr_Q.insert(pair).first;
    }
    return is_collapsed;
}


// Collision Detection

void GameLogics::InitCollisionDetection(std::shared_ptr<cg3d::Model>model1, std::shared_ptr<cg3d::Model> model2, std::shared_ptr<cg3d::Model> cube1, std::shared_ptr<cg3d::Model> cube2) {
    this->model1 = model1;
    this->model2 = model2;
    this->cube1 = cube1;
    this->cube2 = cube2;
}

void GameLogics::AlignedBoxTransformer(Eigen::AlignedBox<double, 3>& aligned_box, std::shared_ptr<cg3d::Model> cube_model)
{
    // Get the aligned box coordinates
    Eigen::RowVector3d BottomRightCeil = aligned_box.corner(aligned_box.BottomRightCeil);
    Eigen::RowVector3d BottomRightFloor = aligned_box.corner(aligned_box.BottomRightFloor);
    Eigen::RowVector3d BottomLeftCeil = aligned_box.corner(aligned_box.BottomLeftCeil);
    Eigen::RowVector3d BottomLeftFloor = aligned_box.corner(aligned_box.BottomLeftFloor);
    Eigen::RowVector3d TopRightCeil = aligned_box.corner(aligned_box.TopRightCeil);
    Eigen::RowVector3d TopRightFloor = aligned_box.corner(aligned_box.TopRightFloor);
    Eigen::RowVector3d TopLeftCeil = aligned_box.corner(aligned_box.TopLeftCeil);
    Eigen::RowVector3d TopLeftFloor = aligned_box.corner(aligned_box.TopLeftFloor);

    // Update the cube with the aligned box coordinates
    Eigen::MatrixXd V, VN, T;
    Eigen::MatrixXi F;

    V.resize(8, 3);
    F.resize(12, 3);
    V.row(0) = BottomLeftFloor;
    V.row(1) = BottomRightFloor;
    V.row(2) = TopLeftFloor;
    V.row(3) = TopRightFloor;
    V.row(4) = BottomLeftCeil;
    V.row(5) = BottomRightCeil;
    V.row(6) = TopLeftCeil;
    V.row(7) = TopRightCeil;
    F.row(0) = Eigen::Vector3i(0, 1, 3);
    F.row(1) = Eigen::Vector3i(3, 2, 0);
    F.row(2) = Eigen::Vector3i(4, 5, 7);
    F.row(3) = Eigen::Vector3i(7, 6, 4);
    F.row(4) = Eigen::Vector3i(0, 4, 6);
    F.row(5) = Eigen::Vector3i(6, 2, 0);
    F.row(6) = Eigen::Vector3i(5, 7, 3);
    F.row(7) = Eigen::Vector3i(7, 3, 1);
    F.row(8) = Eigen::Vector3i(2, 6, 7);
    F.row(9) = Eigen::Vector3i(7, 3, 2);
    F.row(10) = Eigen::Vector3i(4, 5, 1);
    F.row(11) = Eigen::Vector3i(1, 0, 4);

    igl::per_vertex_normals(V, F, VN);
    T = Eigen::MatrixXd::Zero(V.rows(), 2);

    auto mesh = cube_model->GetMeshList();
    mesh[0]->data.push_back({ V, F, VN, T });
    cube_model->SetMeshList(mesh);
    cube_model->meshIndex += 1;
}

bool GameLogics::CollisionCheck(igl::AABB<Eigen::MatrixXd, 3>* object_tree1, igl::AABB<Eigen::MatrixXd, 3>* object_tree2, int level)
{
    // Base cases
    if (object_tree1 == nullptr || object_tree2 == nullptr)
    {
        return false;
    }
    if (!BoxesIntersectionCheck(object_tree1->m_box, object_tree2->m_box))
    {
        return false;
    }

    // Simplification Support
    //if ((level > autoModels[0]->meshIndex) && (level <= max_support)) {
    //    level_down(0);
    //    level_down(1);
    //}

    // If the boxes intersect than creating each object's smallest bounding box
    if (object_tree1->is_leaf() && object_tree2->is_leaf()) {
        AlignedBoxTransformer(object_tree1->m_box, cube1);
        AlignedBoxTransformer(object_tree2->m_box, cube2);
        model1->AddChild(cube1);
        model2->AddChild(cube2);
        cube1->Translate(Eigen::Vector3f(0, 0, 0));
        cube2->Translate(Eigen::Vector3f(0, 0, 0));

        return true;
    }
    if (object_tree1->is_leaf() && !object_tree2->is_leaf()) {
        return CollisionCheck(object_tree1, object_tree2->m_right, level + 1) ||
               CollisionCheck(object_tree1, object_tree2->m_left, level + 1);
    }
    if (!object_tree1->is_leaf() && object_tree2->is_leaf()) {
        return CollisionCheck(object_tree1->m_right, object_tree2, level + 1) ||
               CollisionCheck(object_tree1->m_left, object_tree2, level + 1);
    }
    return CollisionCheck(object_tree1->m_left, object_tree2->m_left, level + 1) ||
           CollisionCheck(object_tree1->m_left, object_tree2->m_right, level + 1) ||
           CollisionCheck(object_tree1->m_right, object_tree2->m_left, level + 1) ||
           CollisionCheck(object_tree1->m_right, object_tree2->m_right, level + 1);
}

bool GameLogics::BoxesIntersectionCheck(Eigen::AlignedBox<double, 3>& aligned_box1, Eigen::AlignedBox<double, 3>& aligned_box2)
{
    // Matrix A
    Eigen::Matrix3d A = model1->GetRotation().cast<double>();
    Eigen::Vector3d A0 = A.col(0);
    Eigen::Vector3d A1 = A.col(1);
    Eigen::Vector3d A2 = A.col(2);

    // Matrix B
    Eigen::Matrix3d B = model2->GetRotation().cast<double>();
    Eigen::Vector3d B0 = B.col(0);
    Eigen::Vector3d B1 = B.col(1);
    Eigen::Vector3d B2 = B.col(2);

    // Matrix C (Where: C=A^T*B)
    Eigen::Matrix3d C = A.transpose() * B;
    // Get the lengths of the sides of the bounding box
    //Eigen::Vector3d a = aligned_box1.sizes();
    //Eigen::Vector3d b = aligned_box2.sizes();
    Eigen::Vector3d a = Eigen::Vector3d(aligned_box1.sizes()[0] * model1->scale_vector[0], aligned_box1.sizes()[1] * model1->scale_vector[1], aligned_box1.sizes()[2] * model1->scale_vector[2]);
    Eigen::Vector3d b = Eigen::Vector3d(aligned_box2.sizes()[0] * model2->scale_vector[0], aligned_box2.sizes()[1] * model2->scale_vector[1], aligned_box2.sizes()[2] * model2->scale_vector[2]);
    a = a / 2;
    b = b / 2;

    // Matrix D
    Eigen::Vector4d CenterA = Eigen::Vector4d(aligned_box1.center()[0], aligned_box1.center()[1], aligned_box1.center()[2], 1);
    Eigen::Vector4d CenterB = Eigen::Vector4d(aligned_box2.center()[0], aligned_box2.center()[1], aligned_box2.center()[2], 1);
    Eigen::Vector4d D4d = model2->GetAggregatedTransform().cast<double>() * CenterB - model1->GetAggregatedTransform().cast<double>() * CenterA;
    Eigen::Vector3d D = D4d.head(3);

    float R0, R1, R;

    // Check the 15 conditions
    // Check A conditions
    // A0
    R0 = a(0);
    R1 = b(0) * abs(C.row(0)(0)) + b(1) * abs(C.row(0)(1)) + b(2) * abs(C.row(0)(2));
    R = abs(A0.transpose() * D);
    if (R0 + R1 < R) return false;
    // A1
    R0 = a(1);
    R1 = b(0) * abs(C.row(1)(0)) + b(1) * abs(C.row(1)(1)) + b(2) * abs(C.row(1)(2));
    R = abs(A1.transpose() * D);
    if (R0 + R1 < R) return false;
    // A2
    R0 = a(2);
    R1 = b(0) * abs(C.row(2)(0)) + b(1) * abs(C.row(2)(1)) + b(2) * abs(C.row(2)(2));
    R = abs(A2.transpose() * D);
    if (R0 + R1 < R) return false;

    // Check B conditions
    // B0
    R0 = a(0) * abs(C.row(0)(0)) + a(1) * abs(C.row(1)(0)) + a(2) * abs(C.row(2)(0));
    R1 = b(0);
    R = abs(B0.transpose() * D);
    if (R0 + R1 < R) return false;
    // B1
    R0 = a(0) * abs(C.row(0)(1)) + a(1) * abs(C.row(1)(1)) + a(2) * abs(C.row(2)(1));
    R1 = b(1);
    R = abs(B1.transpose() * D);
    if (R0 + R1 < R) return false;
    // B2
    R0 = a(0) * abs(C.row(0)(2)) + a(1) * abs(C.row(1)(2)) + a(2) * abs(C.row(2)(2));
    R1 = b(2);
    R = abs(B2.transpose() * D);
    if (R0 + R1 < R) return false;

    // Check A0 conditions
    // A0 X B0
    R0 = a(1) * abs(C.row(2)(0)) + a(2) * abs(C.row(1)(0));
    R1 = b(1) * abs(C.row(0)(2)) + b(2) * abs(C.row(0)(1));
    R = C.row(1)(0) * A2.transpose() * D;
    R -= C.row(2)(0) * A1.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A0 X B1
    R0 = a(1) * abs(C.row(2)(1)) + a(2) * abs(C.row(1)(1));
    R1 = b(0) * abs(C.row(0)(2)) + b(2) * abs(C.row(0)(0));
    R = C.row(1)(1) * A2.transpose() * D;
    R -= C.row(2)(1) * A1.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A0 X B2
    R0 = a(1) * abs(C.row(2)(2)) + a(2) * abs(C.row(1)(2));
    R1 = b(0) * abs(C.row(0)(1)) + b(1) * abs(C.row(0)(0));
    R = C.row(1)(2) * A2.transpose() * D;
    R -= C.row(2)(2) * A1.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;

    // Check A1 conditions
    // A1 X B0
    R0 = a(0) * abs(C.row(2)(0)) + a(2) * abs(C.row(0)(0));
    R1 = b(1) * abs(C.row(1)(2)) + b(2) * abs(C.row(1)(1));
    R = C.row(2)(0) * A0.transpose() * D;
    R -= C.row(0)(0) * A2.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A1 X B1
    R0 = a(0) * abs(C.row(2)(1)) + a(2) * abs(C.row(0)(1));
    R1 = b(0) * abs(C.row(1)(2)) + b(2) * abs(C.row(1)(0));
    R = C.row(2)(1) * A0.transpose() * D;
    R -= C.row(0)(1) * A2.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A1 X B2
    R0 = a(0) * abs(C.row(2)(2)) + a(2) * abs(C.row(0)(2));
    R1 = b(0) * abs(C.row(1)(1)) + b(1) * abs(C.row(1)(0));
    R = C.row(2)(2) * A0.transpose() * D;
    R -= C.row(0)(2) * A2.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;

    // Check A2 conditions
    // A2 X B0
    R0 = a(0) * abs(C.row(1)(0)) + a(1) * abs(C.row(0)(0));
    R1 = b(1) * abs(C.row(2)(2)) + b(2) * abs(C.row(2)(1));
    R = C.row(0)(0) * A1.transpose() * D;
    R -= C.row(1)(0) * A0.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A2 X B1
    R0 = a(0) * abs(C.row(1)(1)) + a(1) * abs(C.row(0)(1));
    R1 = b(0) * abs(C.row(2)(2)) + b(2) * abs(C.row(2)(0));
    R = C.row(0)(1) * A1.transpose() * D;
    R -= C.row(1)(1) * A0.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;
    // A2 X B2
    R0 = a(0) * abs(C.row(1)(2)) + a(1) * abs(C.row(0)(2));
    R1 = b(0) * abs(C.row(2)(1)) + b(1) * abs(C.row(2)(0));
    R = C.row(0)(2) * A1.transpose() * D;
    R -= C.row(1)(2) * A0.transpose() * D;
    R = abs(R);
    if (R0 + R1 < R) return false;

    // All the conditions are met
    return true;
}


// Inverse Kinematics

void GameLogics::InitInverseKinematics(vector<std::shared_ptr<cg3d::Model>> arms, std::shared_ptr<cg3d::Model> destination, bool animate) {
    this->arms = arms;
    this->destination = destination;
    this->animate = animate;
}


// Get the destination position (sphere1 center)
Eigen::Vector3f GameLogics::GetDestinationPosition()
{
    Eigen::Matrix4f destination_transform = destination->GetAggregatedTransform();
    Eigen::Vector3f destination_position = Eigen::Vector3f(destination_transform.col(3).x(), destination_transform.col(3).y(), destination_transform.col(3).z());

    return destination_position;
}

// Get the tip position of arms[link_id]
Eigen::Vector3f GameLogics::GetLinkTipPosition(int link_id)
{
    Eigen::Vector3f cyl_length = Eigen::Vector3f(0, 0, 0.8f);

    Eigen::Matrix4f arm_transform = arms[link_id]->GetAggregatedTransform();
    Eigen::Vector3f arm_center = Eigen::Vector3f(arm_transform.col(3).x(), arm_transform.col(3).y(), arm_transform.col(3).z());
    Eigen::Vector3f arm_tip_position = arm_center + arms[link_id]->GetRotation() * cyl_length;

    return arm_tip_position;
}

// Get the source position of arms[link_id]
Eigen::Vector3f GameLogics::GetLinkSourcePosition(int link_id) {
    Eigen::Vector3f cyl_length = Eigen::Vector3f(0, 0, 0.8f);

    Eigen::Matrix4f arm_transform = arms[link_id]->GetAggregatedTransform();
    Eigen::Vector3f arm_center = Eigen::Vector3f(arm_transform.col(3).x(), arm_transform.col(3).y(), arm_transform.col(3).z());
    Eigen::Vector3f arm_source_position = arm_center - arms[link_id]->GetRotation() * cyl_length;

    return arm_source_position;
}

// Get the euler matrices (A_0, A_1, A_2) according to ZXZ Euler angles
std::vector<Eigen::Matrix3f> GameLogics::GetEulerAnglesMatrices(Eigen::Matrix3f R) {
    // Get phi, theta and psi, according to ZXZ Euler angles
    Eigen::Vector3f zxz = R.eulerAngles(2, 0, 2);

    // Building euler angles matrices
    Eigen::Matrix3f phi;
    phi.row(0) = Eigen::Vector3f(cos(zxz.x()), -sin(zxz.x()), 0);
    phi.row(1) = Eigen::Vector3f(sin(zxz.x()), cos(zxz.x()), 0);
    phi.row(2) = Eigen::Vector3f(0, 0, 1);

    Eigen::Matrix3f theta;
    theta.row(0) = Eigen::Vector3f(1, 0, 0);
    theta.row(1) = Eigen::Vector3f(0, cos(zxz.y()), -sin(zxz.y()));
    theta.row(2) = Eigen::Vector3f(0, sin(zxz.y()), cos(zxz.y()));

    Eigen::Matrix3f psi;
    psi.row(0) = Eigen::Vector3f(cos(zxz.z()), -sin(zxz.z()), 0);
    psi.row(1) = Eigen::Vector3f(sin(zxz.z()), cos(zxz.z()), 0);
    psi.row(2) = Eigen::Vector3f(0, 0, 1);

    std::vector<Eigen::Matrix3f> euler_angles_matrices;
    euler_angles_matrices.push_back(phi);
    euler_angles_matrices.push_back(theta);
    euler_angles_matrices.push_back(psi);

    return euler_angles_matrices;
}

// Inverse Kinematics Coordinate Decent Method
void GameLogics::IKCyclicCoordinateDecentMethod() {
    if (animate_CCD && animate) {
        Eigen::Vector3f D = GetDestinationPosition();
        Eigen::Vector3f first_link_position = GetLinkSourcePosition(first_link_id);

        if ((D - first_link_position).norm() > link_length * num_of_links) {
            std::cout << "cannot reach" << std::endl;
            animate_CCD = false;
            return;
        }

        int curr_link = last_link_id;

        while (curr_link != -1) {
            Eigen::Vector3f R = GetLinkSourcePosition(curr_link);
            Eigen::Vector3f E = GetLinkTipPosition(last_link_id);
            Eigen::Vector3f RD = D - R;
            Eigen::Vector3f RE = E - R;
            float distance = (D - E).norm();

            if (distance < delta) {
                std::cout << "distance: " << distance << std::endl;
                animate_CCD = false;
                return;
            }

            // The plane normal
            Eigen::Vector3f normal = RE.normalized().cross(RD.normalized());

            // Get dot product
            float dot = RD.normalized().dot(RE.normalized());

            // Check that it is between -1 to 1
            if (dot > 1) dot = 1;
            if (dot < -1) dot = -1;

            // Rotate link
            float angle = acosf(dot) / angle_divider;
            Eigen::Vector3f rotation_vector = arms[curr_link]->GetRotation().transpose() * normal;
            Eigen::Matrix3f Ri = (Eigen::AngleAxisf(angle, rotation_vector.normalized())).toRotationMatrix();
            Eigen::Vector3f euler_angles = Ri.eulerAngles(2, 0, 2);

            arms[curr_link]->Rotate(euler_angles[0], Scene::Axis::Z);
            arms[curr_link]->Rotate(euler_angles[1], Scene::Axis::X);
            arms[curr_link]->Rotate(euler_angles[2], Scene::Axis::Z);

            curr_link--;
        }
        animate = false;
    }
}

// Inverse Kinematics Fabrik Method
void GameLogics::IKFabrikMethod() {
    if (animate_Fabrik && animate) {
        // The joint positions
        std::vector<Eigen::Vector3f> p;
        p.resize(num_of_links + 1);

        // The target position
        Eigen::Vector3f t = GetDestinationPosition();

        // The root position
        Eigen::Vector3f root = GetLinkSourcePosition(first_link_id);

        // Set disjoint positions (p_0 the is first disjoin)
        int curr = first_link_id;
        while (curr != num_of_links) {
            p[curr] = GetLinkSourcePosition(curr);
            curr = curr + 1;
        }
        p[last_link_id + 1] = GetLinkTipPosition(last_link_id);

        std::vector<double> ri_array;
        std::vector<double> lambda_i_array;

        ri_array.resize(num_of_links + 1);
        lambda_i_array.resize(num_of_links + 1);

        // 1.1. % The distance between root and target 
        float dist = (root - t).norm();

        // 1.3. % Check whether the target is within reach
        if (dist > link_length * num_of_links) {
            // 1.5. % The target is unreachable
            std::cout << "cannot reach" << std::endl;
            animate_Fabrik = false;
            return;
        }
        else {
            // 1.14. % The target is reachable; thus set as b the initial position of joint p_0
            Eigen::Vector3f b = p[first_link_id];

            // 1.16. % Check wether the distance between the end effector p_n and the target t is greater then a tolerance
            Eigen::Vector3f endEffector = p[last_link_id + 1];
            float diff_A = (endEffector - t).norm();
            float tol = delta;

            if (diff_A < tol) {
                std::cout << "distance: " << diff_A << std::endl;
                animate_Fabrik = false;
                return;
            }
            while (diff_A > tol) {
                // 1.19. % STAGE 1: FORWARD REACHING
                // 1.20. % Set the end effector p_n as target t
                p[last_link_id + 1] = t;
                int parent = last_link_id;
                int child = last_link_id + 1;

                while (parent != -1) {
                    // 1.23. % Find the distance r_i between the new joint position p_i+1 and the joint p_i
                    ri_array[parent] = (p[child] - p[parent]).norm();
                    lambda_i_array[parent] = link_length / ri_array[parent];
                    // 1.26. % Find the new joint positions p_i.
                    p[parent] = (1 - lambda_i_array[parent]) * p[child] + lambda_i_array[parent] * p[parent];
                    child = parent;
                    parent = parent - 1;
                }
                // 1.29. % STAGE 2: BACKWORD REACHING
                // 1.30. % Set the root p0 its initial position
                p[first_link_id] = b;
                parent = first_link_id;
                child = first_link_id + 1;

                while (child != num_of_links) {
                    //1.33. % Find the distance r_i between the new joint position p_i and the joint p_i+1
                    ri_array[parent] = (p[child] - p[parent]).norm();
                    lambda_i_array[parent] = link_length / ri_array[parent];
                    //1.36 % Find the new joint positions p_i.
                    p[child] = (1 - lambda_i_array[parent]) * p[parent] + lambda_i_array[parent] * p[child];
                    parent = child;
                    child = child + 1;
                }
                diff_A = (p[last_link_id + 1] - t).norm();
            }

            // Using Fabrik output to rotate the links
            int curr_link = first_link_id;
            int target_id = first_link_id + 1;

            while (curr_link != num_of_links) {
                IKSolverHelper(curr_link, p[target_id]);
                curr_link = target_id;
                target_id = target_id + 1;
            }

            float distance = (t - GetLinkTipPosition(last_link_id)).norm();

            if (distance < delta) {
                animate_Fabrik = false;
                std::cout << "distance: " << distance << std::endl;
            }
        }
        animate = false;
    }
}

// Inverse Kinematics helper function to perform the links rotations
void GameLogics::IKSolverHelper(int link_id, Eigen::Vector3f D) {
    Eigen::Vector3f R = GetLinkSourcePosition(link_id);
    Eigen::Vector3f E = GetLinkTipPosition(link_id);
    Eigen::Vector3f RD = D - R;
    Eigen::Vector3f RE = E - R;

    // The plane normal
    Eigen::Vector3f normal = RE.normalized().cross(RD.normalized());

    // Get dot product
    float dot = RD.normalized().dot(RE.normalized());

    // Check that it is between -1 to 1
    if (dot > 1) dot = 1;
    if (dot < -1) dot = 1;

    // Rotate link
    float angle = acosf(dot) / angle_divider;
    Eigen::Vector3f rotation_vector = arms[link_id]->GetRotation().transpose() * normal;
    Eigen::Matrix3f Ri = (Eigen::AngleAxisf(angle, rotation_vector.normalized())).toRotationMatrix();
    Eigen::Vector3f euler_angles = Ri.eulerAngles(2, 0, 2);

    arms[link_id]->Rotate(euler_angles[0], Scene::Axis::Z);
    arms[link_id]->Rotate(euler_angles[1], Scene::Axis::X);
    arms[link_id]->Rotate(euler_angles[2], Scene::Axis::Z);
}


// Bezier Curve

void GameLogics::InitBezierCurve(std::shared_ptr<cg3d::Model> model, float stage_size) {
    this->model = model;
    this->stage_size = stage_size;
    t = 0;
}

void GameLogics::GenerateBezierCurve() 
{
    Eigen::Vector3f p0 = GenerateRandomPosition(0);
    Eigen::Vector3f p1 = GenerateRandomPosition(1);
    Eigen::Vector3f p2 = GenerateRandomPosition(2);
    Eigen::Vector3f p3 = GenerateRandomPosition(3);

    Eigen::Matrix <float, 4, 3 > G;

    G.row(0) = p0;
    G.row(1) = p1;
    G.row(2) = p2;
    G.row(3) = p3;

    Eigen::Matrix4f M;

    M << -1, 3, -3, 1,
        3, -6, 3, 0,
        -3, 3, 0, 0,
        1, 0, 0, 0;

    MG = M * G;

    Eigen::MatrixXd vertices(total_curve_points + 1, 3);
    Eigen::MatrixXi faces(total_curve_points, 2);

    for (int i = 0; i <= total_curve_points; i++) {
        Eigen::RowVector4f Ti;
        float t_i = float(i) / total_curve_points;

        Ti[0] = powf(t_i, 3);
        Ti[1] = powf(t_i, 2);
        Ti[2] = t_i;
        Ti[3] = 1;

        Eigen::Vector3f TMG = Ti * MG;
        vertices.row(i) = TMG.cast<double>();

        
        if (i != total_curve_points) {
            faces.row(i) = Eigen::Vector2i(i, i + 1);
        }
    }

    // Create bezier curve model
    Eigen::MatrixXd vertexNormals = Eigen::MatrixXd::Ones(total_curve_points + 1, 3);
    Eigen::MatrixXd textureCoords = Eigen::MatrixXd::Ones(total_curve_points + 1, 2);
    std::shared_ptr<Mesh> coords = std::make_shared<Mesh>("Bezier Curve Mesh", vertices, faces, vertexNormals, textureCoords);

    auto program = std::make_shared<Program>("shaders/phongShader");
    auto material{ std::make_shared<Material>("material", program) };
    material->program->name = "bezier";

    bezier_curve = Model::Create("Bezier Curve", coords, material);
    bezier_curve->mode = 1;

    // Reset model position
    Eigen::Vector3f object_translation = p0 - model->GetTranslation();
    model->Translate(object_translation);
}

void GameLogics::MoveOnCurve() {
    // Find next position
    if (move_forward) {
        if (t > 0.99f) {
            move_forward = false;
            t = 1.f;
        }
        else {
            t += bezier_step;
        }
    }
    else {
        if (t < 0.01f) {
            move_forward = true;
            t = 0.f;
        }
        else {
            t -= bezier_step;
        }
    }

    // Translate to next position
    Eigen::RowVector4f T;

    T[0] = powf(t, 3);
    T[1] = powf(t, 2);
    T[2] = t;
    T[3] = 1;

    Eigen::Vector3f TMG = T * MG;
    Eigen::Vector3f object_translation = TMG - model->GetTranslation();
    model->Translate(object_translation);
}

Eigen::Vector3f GameLogics::GenerateRandomPosition(int point_zone) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_real_distribution<float> distribution1(0, stage_size - 10);
    std::uniform_real_distribution<float> distribution2(-stage_size + 10, 0);
    std::uniform_real_distribution<float> distribution3(-stage_size + 10, stage_size - 10);

    if (point_zone == 0) {
        return Eigen::Vector3f(distribution1(generator), distribution1(generator), distribution3(generator));
    }
    if (point_zone == 1) {
        return Eigen::Vector3f(distribution1(generator), distribution2(generator), distribution3(generator));
    }
    if (point_zone == 2) {
        return Eigen::Vector3f(distribution2(generator), distribution1(generator), distribution3(generator));
    }
    if (point_zone == 3) {
        return Eigen::Vector3f(distribution2(generator), distribution2(generator), distribution3(generator));
    }
}
