#include <glad\glad.h>
#include "BasicScene.h"

#include <GLFW\glfw3.h>
#include "PickVisitor.h"
#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATORS
#include "stb_image.h"
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>


using namespace std;
using namespace cg3d;
using namespace Eigen;


BasicScene::BasicScene(std::string name, Display* display) : SceneWithImGui(std::move(name), display)
{
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;

    // Disable debug messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
}

void BasicScene::Init(float fov, int width, int height, float near, float far)
{
    // Set camera list
    camera_list.resize(camera_list.capacity());
    camera_list[0] = Camera::Create("snake far view", fov, float(width) / height, near, far);
    camera_list[1] = Camera::Create("snake front view", fov, float(width) / height, near, far);
    camera_list[2] = Camera::Create("snake back view", fov, float(width) / height, near, far);
    camera_list[3] = Camera::Create("snake global static view", fov, float(width) / height, near, far);

    camera = camera_list[0];
    number_of_cameras = int(camera_list.size());

    AddChild(root = Movable::Create("root")); // a common (invisible) parent object for all the shapes

    // Create Game Manager
    game_manager = new GameManager();
    game_manager->InitGameManager(root, camera_list);
}

void BasicScene::Update(const Program& program, const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, const Eigen::Matrix4f& model)
{
    Scene::Update(program, proj, view, model);
    
    // If Program Handler finds known program -> skip default setup
    if (ProgramHandler(program)) {
        program.SetUniform4f("lightColor", 0.8f, 0.3f, 0.0f, 0.5f);
        program.SetUniform4f("Kai", 1.0f, 0.3f, 0.6f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.0f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);
    }
}

void BasicScene::MouseCallback(Viewport* viewport, int x, int y, int button, int action, int mods, int buttonState[])
{
    // Handle ImGui Menu
    if (ImGui::GetIO().WantCaptureMouse) return;

    // Handle coming back (In Progress)
    //if (menu_index != StageMenu && menu_index != StageCompletedMenu && menu_index != StageFailedMenu && menu_index != NewHighScoreMenu) {
    //    return;
    //}

    // Reject all mouse callbacks
    if (menu_index != -1) {
        return;
    }

    // note: there's a (small) chance the button state here precedes the mouse press/release event

    if (action == GLFW_PRESS) { // default mouse button press behavior
        PickVisitor visitor;
        visitor.Init();
        renderer->RenderViewportAtPos(x, y, &visitor); // pick using fixed colors hack
        auto modelAndDepth = visitor.PickAtPos(x, renderer->GetWindowHeight() - y);
        renderer->RenderViewportAtPos(x, y); // draw again to avoid flickering
        pickedModel = modelAndDepth.first ? std::dynamic_pointer_cast<Model>(modelAndDepth.first->shared_from_this()) : nullptr;
        pickedModelDepth = modelAndDepth.second;
        camera->GetRotation().transpose();
        xAtPress = x;
        yAtPress = y;

        // if (pickedModel)
        //     debug("found ", pickedModel->isPickable ? "pickable" : "non-pickable", " model at pos ", x, ", ", y, ": ",
        //           pickedModel->name, ", depth: ", pickedModelDepth);
        // else
        //     debug("found nothing at pos ", x, ", ", y);

        if (pickedModel && !pickedModel->isPickable)
            pickedModel = nullptr; // for non-pickable models we need only pickedModelDepth for mouse movement calculations later

        if (pickedModel)
            pickedToutAtPress = pickedModel->GetTout();
        else
            cameraToutAtPress = camera->GetTout();
    }
}

void BasicScene::ScrollCallback(Viewport* viewport, int x, int y, int xoffset, int yoffset, bool dragging, int buttonState[])
{
    //// note: there's a (small) chance the button state here precedes the mouse press/release event
    //auto system = camera->GetRotation().transpose();
    //if (pickedModel) {
    //    pickedModel->TranslateInSystem(system, { 0, 0, -float(yoffset) });
    //    pickedToutAtPress = pickedModel->GetTout();
    //}
    //else {
    //    camera->TranslateInSystem(system, { 0, 0, -float(yoffset) });
    //    cameraToutAtPress = camera->GetTout();
    //}

    // Handle ImGui Menu
    if (ImGui::GetIO().WantCaptureMouse) return;

    // Enable scrolling only for global cameras
    if (camera_index == 0) {
        camera->Translate({ 0, 0, -float(yoffset) });
        cameraToutAtPress = camera->GetTout();
    }
    else if (camera_index == 3) {
        camera->Translate({ 0, -float(yoffset), 0 });
        cameraToutAtPress = camera->GetTout();
    }
}

void BasicScene::CursorPosCallback(Viewport* viewport, int x, int y, bool dragging, int* buttonState)
{
    // Handle ImGui Menu
    if (ImGui::GetIO().WantCaptureMouse) return;

    //if (dragging) {
    //    auto system = camera->GetRotation().transpose() * GetRotation();
    //    auto moveCoeff = camera->CalcMoveCoeff(pickedModelDepth, viewport->width);
    //    auto angleCoeff = camera->CalcAngleCoeff(viewport->width);
    //    if (pickedModel) {
    //        //pickedModel->SetTout(pickedToutAtPress);
    //        if (buttonState[GLFW_MOUSE_BUTTON_RIGHT] != GLFW_RELEASE)
    //            pickedModel->TranslateInSystem(system, {-float(xAtPress - x) / moveCoeff, float(yAtPress - y) / moveCoeff, 0});
    //        if (buttonState[GLFW_MOUSE_BUTTON_MIDDLE] != GLFW_RELEASE)
    //            pickedModel->RotateInSystem(system, float(xAtPress - x) / angleCoeff, Axis::Z);
    //        if (buttonState[GLFW_MOUSE_BUTTON_LEFT] != GLFW_RELEASE) {
    //            pickedModel->RotateInSystem(system, float(xAtPress - x) / angleCoeff, Axis::Y);
    //            pickedModel->RotateInSystem(system, float(yAtPress - y) / angleCoeff, Axis::X);
    //        }
    //    } else {
    //       // camera->SetTout(cameraToutAtPress);
    //        if (buttonState[GLFW_MOUSE_BUTTON_RIGHT] != GLFW_RELEASE)
    //            root->TranslateInSystem(system, {-float(xAtPress - x) / moveCoeff/10.0f, float( yAtPress - y) / moveCoeff/10.0f, 0});
    //        if (buttonState[GLFW_MOUSE_BUTTON_MIDDLE] != GLFW_RELEASE)
    //            root->RotateInSystem(system, float(x - xAtPress) / 180.0f, Axis::Z);
    //        if (buttonState[GLFW_MOUSE_BUTTON_LEFT] != GLFW_RELEASE) {
    //            root->RotateInSystem(system, float(x - xAtPress) / angleCoeff, Axis::Y);
    //            root->RotateInSystem(system, float(y - yAtPress) / angleCoeff, Axis::X);
    //        }
    //    }
    //    xAtPress =  x;
    //    yAtPress =  y;
    //}

    if ((dragging) && (camera_list[0] == camera)) {
        //auto system = camera->GetRotation().transpose() * GetRotation();
        auto system = Eigen::Matrix3f::Identity();;
        auto moveCoeff = camera->CalcMoveCoeff(pickedModelDepth, viewport->width);
        auto angleCoeff = camera->CalcAngleCoeff(viewport->width);
        
        if (buttonState[GLFW_MOUSE_BUTTON_RIGHT] != GLFW_RELEASE) {
            camera->RotateInSystem(system, float(y - yAtPress) / angleCoeff, Axis::X);
        }
        if (buttonState[GLFW_MOUSE_BUTTON_MIDDLE] != GLFW_RELEASE) {
            camera->RotateInSystem(system, float(x - xAtPress) / 180.0f, Axis::Z);
        }
        if (buttonState[GLFW_MOUSE_BUTTON_LEFT] != GLFW_RELEASE) {
            camera->RotateInSystem(system, float(x - xAtPress) / angleCoeff, Axis::Y);
        }

        xAtPress = x;
        yAtPress = y;
    }
}

void BasicScene::KeyCallback(Viewport* viewport, int x, int y, int key, int scancode, int action, int mods)
{
    // Handle ImGui Menu
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    static bool WASD_events[4];
    static bool UDLR_events[4];

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) // NOLINT(hicpp-multiway-paths-covered)
        {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_W:
                WASD_events[0] = true;
                break;
            case GLFW_KEY_S:
                WASD_events[1] = true;
                break;
            case GLFW_KEY_A:
                WASD_events[2] = true;
                break;
            case GLFW_KEY_D:
                WASD_events[3] = true;
                break;
            case GLFW_KEY_UP:
                UDLR_events[0] = true;
                break;
            case GLFW_KEY_DOWN:
                UDLR_events[1] = true;
                break;
            case GLFW_KEY_LEFT:
                UDLR_events[2] = true;
                break;
            case GLFW_KEY_RIGHT:
                UDLR_events[3] = true;
                break;
            case GLFW_KEY_V:
                SwitchView(true);
                break;
            case GLFW_KEY_B:
                SwitchView(false);
                break;
            case GLFW_KEY_SPACE:
                if (display_loading == false) {
                    if (menu_index == StageMenu)
                    {
                        animate = false;
                        game_manager->PauseStageHandle(true);
                        menu_index = PauseMenu;
                    }
                    else if (menu_index == PauseMenu)
                    {
                        animate = true;
                        game_manager->PauseStageHandle(false);
                        menu_index = StageMenu;
                    }
                }
                break;
            case GLFW_KEY_R:
                game_manager->snake.ResetCameraView();
                break;

            // Debug Mode Buttons
            case GLFW_KEY_1:
                debug_parameter = 1;
                cout << "Health Debug" << endl;
                break;
            case GLFW_KEY_2:
                debug_parameter = 2;
                cout << "Score Debug" << endl;
                break;
            case GLFW_KEY_3:
                debug_parameter = 3;
                cout << "Gold Debug" << endl;
                break;
            case GLFW_KEY_EQUAL:
                if (debug_parameter == 1)
                {
                    game_manager->stats->current_health += 10;
                    game_manager->stats->total_health_points_healed += 10;
                }
                if (debug_parameter == 2)
                {
                    game_manager->stats->current_score += 10;
                    game_manager->stats->total_score_points_earned += 10;
                }
                if (debug_parameter == 3)
                {
                    game_manager->stats->gold += 10;
                    game_manager->stats->total_gold_earned += 10;
                }
                cout << "+ Debug" << endl;
                break;
            case GLFW_KEY_MINUS:
                if (debug_parameter == 1)
                {
                    game_manager->stats->current_health -= 10;
                    game_manager->stats->total_health_points_lost += 10;
                }
                if (debug_parameter == 2)
                {
                    game_manager->stats->current_score -= 10;
                    // No event
                }
                if (debug_parameter == 3)
                {
                    game_manager->stats->gold -= 10;
                    game_manager->stats->total_gold_spent += 10;
                }
                cout << "- Debug" << endl;
                break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_W:
                WASD_events[0] = false;
                break;
            case GLFW_KEY_S:
                WASD_events[1] = false;
                break;
            case GLFW_KEY_A:
                WASD_events[2] = false;
                break;
            case GLFW_KEY_D:
                WASD_events[3] = false;
                break;
            case GLFW_KEY_UP:
                UDLR_events[0] = false;
                break;
            case GLFW_KEY_DOWN:
                UDLR_events[1] = false;
                break;
            case GLFW_KEY_LEFT:
                UDLR_events[2] = false;
                break;
            case GLFW_KEY_RIGHT:
                UDLR_events[3] = false;
                break;
        }
    }

    if (animate)
    {
        // WASD events
        if (WASD_events[0]) 
        {
            game_manager->snake.MoveUp();
        }
        if (WASD_events[1])
        {
            game_manager->snake.MoveDown();
        }
        if (WASD_events[2])
        {
            game_manager->snake.MoveLeft();
        }
        if (WASD_events[3])
        {
            game_manager->snake.MoveRight();
        }

        // UP DWON LEFT RIGHT events
        if (UDLR_events[0])
        {
            if (game_manager->stats->current_movement_speed < game_manager->stats->max_movement_speed)
            {
                game_manager->stats->current_movement_speed += 1;
            }
            else
            {
                cout << "Max movement speed already reached" << endl;
            }
        }
        if (UDLR_events[1])
        {
            if (game_manager->stats->current_movement_speed > game_manager->stats->min_movement_speed)
            {
                game_manager->stats->current_movement_speed -= 1;
            }
            else
            {
                cout << "Min movement speed already reached" << endl;
            }
        }
        if (UDLR_events[2])
        {
            game_manager->snake.RollLeft();
        }
        if (UDLR_events[3])
        {
            game_manager->snake.RollRight();
        }
    }
}

void BasicScene::BuildImGui() {
    MenuManager();
}

void BasicScene::SetCamera(int index)
{
    camera = camera_list[index];
    viewport->camera = camera;
}

void BasicScene::ViewportSizeCallback(Viewport* _viewport)
{
    for (auto& cam : camera_list) {
        cam->SetProjection(float(_viewport->width) / float(_viewport->height));
    }

    // note: we don't need to call Scene::ViewportSizeCallback since we are setting the projection of all the cameras
}

void BasicScene::AddViewportCallback(Viewport* _viewport)
{
    viewport = _viewport;

    Scene::AddViewportCallback(viewport);
}


void BasicScene::SwitchView(bool next)
{
    if (next) {
        camera_index = (camera_index + 1) % number_of_cameras;
    }
    else {
        camera_index = (camera_index + 3) % number_of_cameras;
    }
    
    camera = camera_list[camera_index];
    viewport->camera = camera;
}


void BasicScene::MenuManager() {
    width = viewport->width;
    height = viewport->height;

    switch (menu_index)
    {
        case LoginMenu:
            LoginMenuHandler();
            break;
        case MainMenu:
            MainMenuHandler();
            break;
        case ShopMenu:
            ShopMenuHandler();
            break;
        case StatsMenu:
            StatsMenuHandler();
            break;
        case StageSelectionMenu:
            StageSelectionMenuHandler();
            break;
        case HallOfFameMenu:
            HallOfFameMenuHandler();
            break;
        case SettingsMenu:
            SettingsMenuHandler();
            break;
        case CreditsMenu:
            CreditsMenuHandler();
            break;
        case StageMenu:
            StageMenuHandler();
            break;
        case PauseMenu:
            PauseMenuHandler();
            break;
        case StageCompletedMenu:
            StageCompletedMenuHandler();
            break;
        case StageFailedMenu:
            StageFailedMenuHandler();
            break;
        case NewHighScoreMenu:
            NewHighScoreMenuHandler();
            break;
    }
}

void BasicScene::LoginMenuHandler() {
    if (game_manager->sound_manager->playing_index != MainMenu) {
        game_manager->sound_manager->HandleMusic("opening_theme.mp3");
        game_manager->sound_manager->playing_index = MainMenu;

        SetMenuImage("main_menu.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, input_text_size1;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 3.f, float(height) / 6.f);

        input_text_size1 = ImVec2((float(width) / 3.f), (float(height) / 16.f));

        font_scale1 = float(width) / 400.f;

        text_position1 = float(width) * 0.38f;
        text_position2 = float(width) * 0.3f;
    }
    else {
        window_size1 = buttons_size1 = input_text_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "S");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "n");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), "a");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "k");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "e");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), " 3");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "D");

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (display_new_game) {
        if (ImGui::Button("New Game", buttons_size1)) {
            display_new_game = false;
            Login_InvalidSaveData = false;
        }
    }
    else {
        ImGui::Text("Your Name: ");

        ImGui::SetCursorPosX(text_position2);
        static char name[21] = "";
        ImGui::InputTextMultiline("", name, IM_ARRAYSIZE(name), input_text_size1);

        Spacing(5);
        
        ImGui::SetCursorPosX(text_position2);
        if (ImGui::Button("Start New Game", buttons_size1)) {
            if (name[0] == '\0') {
                Login_InvalidName = true;
            }
            else {
                display_new_game = true;
                Login_InvalidName = false;
                Login_InvalidSaveData = false;
                menu_index = MainMenu;

                game_manager->NewGame(name);
            }
        }
    }

    if (Login_InvalidName) {
        ImGui::SetCursorPosX(text_position2);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Invalid input name!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (display_new_game) {
        if (!game_manager->stats->save_data_available) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            // If save data not found
            if (ImGui::Button("Continue Game", buttons_size1)) {
                Login_InvalidSaveData = true;
            }
            ImGui::PopStyleColor();
        }
        else {
            if (ImGui::Button("Continue Game", buttons_size1)) {
                // If save data found
                display_new_game = true;
                Login_InvalidSaveData = false;
                menu_index = MainMenu;
            }
        }
    }
    else {
        if (ImGui::Button("Cancel", buttons_size1)) {
            display_new_game = true;
            Login_InvalidName = false;
        }
    }

    if (Login_InvalidSaveData) {
        ImGui::SetCursorPosX(text_position2);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "No save data available!");
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Exit", buttons_size1)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    ImGui::End();
}

void BasicScene::MainMenuHandler() {
    if (game_manager->sound_manager->playing_index != MainMenu) {
        game_manager->sound_manager->HandleMusic("opening_theme.mp3");
        game_manager->sound_manager->playing_index = MainMenu;

        SetMenuImage("main_menu.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, buttons_size2;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 9.f);
        buttons_size2 = ImVec2(float(width) / 4.f, float(height) / 15.f);

        font_scale1 = float(width) / 600.f;

        text_position1 = float(width) * 0.42f;
        text_position2 = float(width) * 0.35f;
    }
    else {
        window_size1 = buttons_size1 = buttons_size2 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Main Menu");

    Spacing(5);

    // Handel User name
    gui_text = "Welcome back, " + game_manager->stats->user_name;
    ImGui::SetCursorPosX(text_position2);
    ImGui::Text(gui_text.c_str());

    Spacing(5);

    // Handle Gold
    gui_text = "Gold: " + std::to_string(game_manager->stats->gold);
    ImGui::SetCursorPosX(text_position2);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Select Stage", buttons_size2)) {
        menu_index = StageSelectionMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Save Game", buttons_size2)) {
        game_manager->SaveGame();
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Shop", buttons_size2)) {
        last_menu_index = MainMenu;
        menu_index = ShopMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Stats", buttons_size2)) {
        menu_index = StatsMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Hall Of Fame", buttons_size2)) {
        menu_index = HallOfFameMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Settings", buttons_size2)) {
        last_menu_index = MainMenu;
        menu_index = SettingsMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Credits", buttons_size2)) {
        menu_index = CreditsMenu;
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Logout", buttons_size1)) {
        game_manager->stats->InitStats();
        game_manager->leaderboard->InitLeaderboard();
        menu_index = LoginMenu;
    }
    
    ImGui::End();
}

void BasicScene::StageSelectionMenuHandler() {
    if (game_manager->sound_manager->playing_index != MainMenu) {
        game_manager->sound_manager->HandleMusic("opening_theme.mp3");
        game_manager->sound_manager->playing_index = MainMenu;

        SetMenuImage("main_menu.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1;
    float font_scale1, font_scale2, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 8.f);

        font_scale1 = float(width) / 400.f;

        text_position1 = float(width) * 0.35f;
        text_position2 = float(width) * 0.3f;
    }
    else {
        window_size1 = buttons_size1 = ImVec2(1, 1);
        font_scale1 = font_scale2 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position2);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Stage Selection Menu");

    Spacing(5);

    // Handle Gold
    ImGui::SetCursorPosX(text_position1);
    gui_text = "Gold: " + std::to_string(game_manager->stats->gold);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    // Handle Stages
    ImGui::SetCursorPosX(text_position1);
    ImGui::Text("Select Stage: ");

    // Stages buttons
    for (int i = 1; i <= 3; i++) {
        Spacing(5);

        ImGui::SetCursorPosX(text_position1);
        gui_text = "Stage " + std::to_string(i);

        if (ImGui::Button(gui_text.c_str(), buttons_size1)) {
            cout << gui_text.c_str() << endl;
            game_manager->sound_manager->stage_index = i;
            menu_index = StageMenu;

            game_manager->LoadStage(i, true);
            SetMenuImage("loading.jpg");
            display_loading = true;
        }
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position1);
    if (ImGui::Button("Back", buttons_size1)) {
        menu_index = MainMenu;
    }
    ImGui::End();
}

void BasicScene::ShopMenuHandler() {
    if (game_manager->sound_manager->playing_index != ShopMenu) {
        game_manager->sound_manager->HandleMusic("shop.mp3");
        game_manager->sound_manager->playing_index = ShopMenu;

        SetMenuImage("shop.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, buttons_size2;
    float font_scale1, text_position1, text_position2, text_position3;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 9.f);
        buttons_size2 = ImVec2(float(width) / 4.f, float(height) / 15.f);

        font_scale1 = float(width) / 600.f;

        text_position1 = float(width) * 0.44f;
        text_position2 = float(width) * 0.35f;
        text_position3 = float(width) * 0.15f;
    }
    else {
        window_size1 = buttons_size1 = buttons_size2 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = text_position3 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;
    int item1_cost, item2_cost, item3_cost, item4_cost, item5_cost;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Shop");

    Spacing(5);

    // Handle Gold
    ImGui::SetCursorPosX(text_position2);
    int gold = game_manager->stats->gold;
    gui_text = "Gold: " + std::to_string(gold);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    int max_health = game_manager->stats->max_health;
    gui_text = "Increase Max Health (Current: " + to_string(max_health) + ")";
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SetCursorPosX(text_position2);
    item1_cost = max_health;
    gui_text = to_string(item1_cost) + " Gold###1";
    if (ImGui::Button(gui_text.c_str(), buttons_size2)) {
        if (gold >= item1_cost) {
            game_manager->stats->max_health += 10;
            game_manager->stats->gold -= item1_cost;
            game_manager->stats->total_gold_spent += item1_cost;
            cout << "Increase Max Health" << endl;
            ShopMenu_InvalidGold1 = false;
        }
        else {
            ShopMenu_InvalidGold1 = true;
            ShopMenu_InvalidGold2 = false;
            ShopMenu_InvalidGold3 = false;
            ShopMenu_InvalidGold4 = false;
            ShopMenu_InvalidGold5 = false;
        }
    }
    if (ShopMenu_InvalidGold1) {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "You don't have enoguh gold!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    int score_multiplier = game_manager->stats->score_multiplier;
    gui_text = "X" + to_string(score_multiplier + 1) + " Score (Current: X" + to_string(score_multiplier) + ")";
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SetCursorPosX(text_position2);
    item2_cost = score_multiplier * 10;
    gui_text = to_string(item2_cost) + " Gold###2";
    if (ImGui::Button(gui_text.c_str(), buttons_size2)) {
        if (gold >= item2_cost) {
            game_manager->stats->score_multiplier++;
            game_manager->stats->gold -= item2_cost;
            game_manager->stats->total_gold_spent += item2_cost;
            cout << "Increase X2 Score" << endl;
            ShopMenu_InvalidGold2 = false;
        }
        else {
            ShopMenu_InvalidGold1 = false;
            ShopMenu_InvalidGold2 = true;
            ShopMenu_InvalidGold3 = false;
            ShopMenu_InvalidGold4 = false;
            ShopMenu_InvalidGold5 = false;
        }
    }
    if (ShopMenu_InvalidGold2) {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "You don't have enoguh gold!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    int gold_multiplier = game_manager->stats->gold_multiplier;
    gui_text = "X" + to_string(gold_multiplier + 1) + " Gold (Current: X" + to_string(gold_multiplier) + ")";
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SetCursorPosX(text_position2);
    item3_cost = gold_multiplier * 10;
    gui_text = to_string(item3_cost) + " Gold###3";
    if (ImGui::Button(gui_text.c_str(), buttons_size2)) {
        if (gold >= item3_cost) {
            game_manager->stats->gold_multiplier++;
            game_manager->stats->gold -= item3_cost;
            game_manager->stats->total_gold_spent += item3_cost;
            cout << "Increase X2 Gold" << endl;
            ShopMenu_InvalidGold3 = false;
        }
        else {
            ShopMenu_InvalidGold1 = false;
            ShopMenu_InvalidGold2 = false;
            ShopMenu_InvalidGold3 = true;
            ShopMenu_InvalidGold4 = false;
            ShopMenu_InvalidGold5 = false;
        }
    }
    if (ShopMenu_InvalidGold3) {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "You don't have enoguh gold!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    int bonuses_duration = game_manager->stats->bonuses_duration;
    gui_text = "Increase Bonuses Duration (Current: " + to_string(bonuses_duration) + " sec)";
    ImGui::TextColored(ImVec4(1.0, 0.0, 1.0, 1.0), gui_text.c_str());

    ImGui::SetCursorPosX(text_position2);
    item4_cost = bonuses_duration * 10;
    gui_text = to_string(item4_cost) + " Gold###4";
    if (ImGui::Button(gui_text.c_str(), buttons_size2)) {
        if (gold >= item4_cost) {
            game_manager->stats->bonuses_duration++;
            game_manager->stats->gold -= item4_cost;
            game_manager->stats->total_gold_spent += item4_cost;
            cout << "Increase Bonuses Duration" << endl;
            ShopMenu_InvalidGold4 = false;
        }
        else {
            ShopMenu_InvalidGold1 = false;
            ShopMenu_InvalidGold2 = false;
            ShopMenu_InvalidGold3 = false;
            ShopMenu_InvalidGold4 = true;
            ShopMenu_InvalidGold5 = false;
        }
    }
    if (ShopMenu_InvalidGold4) {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "You don't have enoguh gold!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    int max_movement_speed = game_manager->stats->max_movement_speed;
    gui_text = "Increase Max Speed (Current: " + to_string(max_movement_speed) + ")";
    ImGui::TextColored(ImVec4(0.0, 1.0, 1.0, 1.0), gui_text.c_str());

    ImGui::SetCursorPosX(text_position2);
    item5_cost = max_movement_speed * 100;
    gui_text = to_string(item5_cost) + " Gold###5";
    if (ImGui::Button(gui_text.c_str(), buttons_size2)) {
        if (gold >= item5_cost) {
            game_manager->stats->max_movement_speed++;
            game_manager->stats->gold -= item5_cost;
            game_manager->stats->total_gold_spent += item5_cost;
            cout << "Increase Max Speed" << endl;
            ShopMenu_InvalidGold5 = false;
        }
        else {
            ShopMenu_InvalidGold1 = false;
            ShopMenu_InvalidGold2 = false;
            ShopMenu_InvalidGold3 = false;
            ShopMenu_InvalidGold4 = false;
            ShopMenu_InvalidGold5 = true;
        }
    }
    if (ShopMenu_InvalidGold5) {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "You don't have enoguh gold!");
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back", buttons_size1)) {
        ShopMenu_InvalidGold1 = false;
        ShopMenu_InvalidGold2 = false;
        ShopMenu_InvalidGold3 = false;
        ShopMenu_InvalidGold4 = false;
        ShopMenu_InvalidGold5 = false;

        menu_index = last_menu_index;
    }

    ImGui::End();
}

void BasicScene::StatsMenuHandler() {
    if (game_manager->sound_manager->playing_index != StatsMenu) {
        game_manager->sound_manager->HandleMusic("stats.mp3");
        game_manager->sound_manager->playing_index = StatsMenu;

        SetMenuImage("stats.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, buttons_size2;
    float font_scale1, text_position1, text_position2, text_position3;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 8.f);
        buttons_size2 = ImVec2(float(width) / 4.f, float(height) / 14.f);

        font_scale1 = float(width) / 550.f;

        text_position1 = float(width) * 0.43f;
        text_position2 = float(width) * 0.35f;
        text_position3 = float(width) * 0.3f;

    }
    else {
        window_size1 = buttons_size1 = buttons_size2 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = text_position3 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Stats");

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "Snake Stats");

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Max Health: " + std::to_string(game_manager->stats->max_health);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Score Multiplier: X" + std::to_string(game_manager->stats->score_multiplier);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Gold Multiplier: X" + std::to_string(game_manager->stats->gold_multiplier);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Bonuses Duration: " + std::to_string(game_manager->stats->bonuses_duration) + " sec";
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Max Movement Speed: " + std::to_string(game_manager->stats->max_movement_speed);
    ImGui::Text(gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Statistics");

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Health Points Healed: " + std::to_string(game_manager->stats->total_health_points_healed);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Health Points Lost: " + std::to_string(game_manager->stats->total_health_points_lost);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Score Points Earned: " + std::to_string(game_manager->stats->total_score_points_earned);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Gold Earned: " + std::to_string(game_manager->stats->total_gold_earned);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Gold Spent: " + std::to_string(game_manager->stats->total_gold_spent);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Bonuses Collected: " + std::to_string(game_manager->stats->total_bonuses_collected);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    gui_text = "Total Deaths: " + std::to_string(game_manager->stats->total_deaths);
    ImGui::Text(gui_text.c_str());

    ImGui::SetCursorPosX(text_position3);
    float time_played = game_manager->game_timer.GetElapsedTime();
    gui_text = "Time Played: " + game_manager->game_timer.SecondsToGameTime(time_played);
    ImGui::Text(gui_text.c_str());

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back", buttons_size1)) {
        menu_index = MainMenu;
    }

    ImGui::End();
}

void BasicScene::HallOfFameMenuHandler() {
    if (game_manager->sound_manager->playing_index != HallOfFameMenu) {
        game_manager->sound_manager->HandleMusic("hall_of_fame.mp3");
        game_manager->sound_manager->playing_index = HallOfFameMenu;

        SetMenuImage("hall_of_fame.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 8.f);

        font_scale1 = float(width) / 400.f;

        text_position1 = float(width) * 0.35f;
        text_position2 = float(width) * 0.3f;
    }
    else {
        window_size1 = buttons_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Hall of Fame");

    Spacing(5);

    for (int i = 0; i < 10; i++) {
        ImGui::SetCursorPosX(text_position2);
        string space = "  ";
        if (i == 9) {
            space = " ";
        }
        gui_text = to_string(i + 1) + "." + space +                          // Place
            game_manager->leaderboard->leaderboard_list[i].first + " - " +    // Name
            to_string(game_manager->leaderboard->leaderboard_list[i].second); // Score

        
        if (i == 0) {
            ImGui::TextColored(ImVec4(201.f / 176.f, 149.f / 255.f, 55.f / 255.f, 1.0), gui_text.c_str());
        }
        else if (i == 1) {
            ImGui::TextColored(ImVec4(180.f / 255.f, 180.f / 255.f, 180.f / 255.f, 1.0), gui_text.c_str());
        }
        else if (i == 2) {
            ImGui::TextColored(ImVec4(173.f / 255.f, 138.f / 255.f, 86.f / 255.f, 1.0), gui_text.c_str());
        }
        else {
            ImGui::Text(gui_text.c_str());
        }
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position1);
    if (ImGui::Button("Back", buttons_size1)) {
        menu_index = MainMenu;
    }

    ImGui::End();
}

void BasicScene::SettingsMenuHandler() {
    if (game_manager->sound_manager->playing_index != SettingsMenu) {
        game_manager->sound_manager->HandleMusic("settings.mp3");
        game_manager->sound_manager->playing_index = SettingsMenu;

        SetMenuImage("settings.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, buttons_size2;
    float font_scale1, text_position1, text_position2, text_position3;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 9.f);
        buttons_size2 = ImVec2(float(width) / 4.f, float(height) / 15.f);

        font_scale1 = float(width) / 600.f;

        text_position1 = float(width) * 0.42f;
        text_position2 = float(width) * 0.35f;
        text_position3 = float(width) * 0.15f;
    }
    else {
        window_size1 = buttons_size1 = buttons_size2 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = text_position3 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Settings");

    Spacing(5);

    if (last_menu_index != PauseMenu) {
        bool* skinning_enabled = &game_manager->snake.skinning_enabled;
        if (*skinning_enabled)
            gui_text = "Skinning On";
        else {
            gui_text = "Skinning Off";
        }

        ImGui::SetCursorPosX(text_position2);
        if (ImGui::Checkbox(gui_text.c_str(), skinning_enabled)) {
            if (*skinning_enabled) {
                cout << "Skinning enabled" << endl;
            }
            else {
                cout << "Skinning disabled" << endl;
            }
        }
    }
    else {
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Skinning option is blocked during stage!");
    }

    Spacing(5);

    bool* music_player_enabled = &game_manager->sound_manager->music_player_enabled;
    if (*music_player_enabled)
        gui_text = "Music On";
    else {
        gui_text = "Music Off";
    }

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Checkbox(gui_text.c_str(), music_player_enabled)) {
        if (*music_player_enabled) {
            game_manager->sound_manager->playing_index = -1;
            cout << "Music enabled" << endl;
        }
        else {
            game_manager->sound_manager->StopMusic();
            cout << "Music disabled" << endl;
        }

    }

    Spacing(5);

    bool* sound_player_enabled = &game_manager->sound_manager->sound_player_enabled;
    if (*sound_player_enabled)
        gui_text = "Sound On";
    else {
        gui_text = "Sound Off";
    }

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Checkbox(gui_text.c_str(), sound_player_enabled)) {
        if (*sound_player_enabled) {
            cout << "Sound enabled" << endl;
        }
        else {
            cout << "Sound disabled" << endl;
        }
    }

    Spacing(5);
    
    if (ImGui::SliderFloat("Music Volume", &game_manager->sound_manager->music_volume, 0.0f, 1.0f)) {
        game_manager->sound_manager->UpdateMusicPlayerVolume();
        cout << "Music volume: " + to_string(game_manager->sound_manager->music_volume) << endl;
    }

    Spacing(5);

    if (ImGui::SliderFloat("Sound Volume", &game_manager->sound_manager->sound_volume, 0.0f, 1.0f)) {
        cout << "Sound volume: " + to_string(game_manager->sound_manager->sound_volume) << endl;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Test Sound", buttons_size2)) {
        game_manager->sound_manager->HandleSound("obstacle_object.mp3");
    }

    Spacing(5);

    static float rgb[3] = { snake_color.x(), snake_color.y(), snake_color.z() };
    ImGui::ColorEdit3("Snake Color", rgb);
    snake_color = Eigen::Vector3f(rgb[0], rgb[1], rgb[2]);

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (!display_keys) {
        if (ImGui::Button("Show Keys", buttons_size2)) {
            display_keys = true;
        }
    }
    else {
        if (ImGui::Button("Hide Keys", buttons_size2)) {
            display_keys = false;
        }
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Keyboard Keys: ");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "W - Move snake up");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "S - Move snake down");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "A - Move snake left");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "D - Move snake right");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "V - Switch view forward");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "B - Switch view backward");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "R - Reset global view");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "UP - Increase snake speed");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "DOWN - Decrease snake speed");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "LEFT - Rotate snake left");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "RIGHT - Rotate snake right");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "SPACE - Pause stage");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "ESC - Exit game");
        Spacing(5);
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Mouse Keys: ");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Right click hold - Move snake far view left and right");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Left click hold - Move snake far view up and down");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Scroll click hold - Roll snake far view left and right");
        ImGui::SetCursorPosX(text_position3);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Scroll - Zoom snake global and far view in and out");
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back", buttons_size1)) {
        display_keys = false;
        menu_index = last_menu_index;
    }

    ImGui::End();
}

void BasicScene::CreditsMenuHandler() {
    if (game_manager->sound_manager->playing_index != CreditsMenu) {
        game_manager->sound_manager->HandleMusic("credits.mp3");
        game_manager->sound_manager->playing_index = CreditsMenu;

        SetMenuImage("credits.jpg");
    }

    // Set sizes
    ImVec2 window_size1, buttons_size1, buttons_size2;
    float font_scale1, text_position1, text_position2, text_position3;

    if (width != 0 && height != 0) {
        window_size1 = ImVec2(float(width), float(height));

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 8.f);
        buttons_size2 = ImVec2(float(width) / 2.f, float(height) / 10.f);

        font_scale1 = float(width) / 250.f;

        text_position1 = float(width) * 0.38f;
        text_position2 = float(width) * 0.35f;
        text_position3 = float(width) * 0.05f;
    }
    else {
        window_size1 = buttons_size1 = buttons_size2 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = text_position3 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.6, 1.0, 0.4, 1.0), "Credits");

    Spacing(10);

    ImGui::SetCursorPosX(text_position3);
    ImGui::Text("Created by: Ofir Gilad");

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    ImGui::Text("Course Teacher: Tamir Grossinger");

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    ImGui::Text("Game Engine: OpenGL");

    Spacing(5);

    ImGui::SetCursorPosX(text_position3);
    if (payed_credits) {
        gui_text = "Honorable Contributor: " + game_manager->stats->user_name;
        ImGui::Text(gui_text.c_str());
    }
    else {
        if (ImGui::Button("Contribute Credit", buttons_size2)) {
            payed_credits = true;
        }
    }

    Spacing(10);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back", buttons_size1)) {
        menu_index = MainMenu;
    }

    ImGui::End();
}

void BasicScene::StageMenuHandler() {
    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground;
    bool* pOpen = nullptr;
    string gui_text;

    // Set sizes
    ImVec2 window_position1, window_size1, window_size2, buttons_size1;
    float font_scale1, font_scale2, text_position1;

    if (width != 0 && height != 0) {
        window_position1 = ImVec2(float(width) * 0.45f, float(height) * 0.38f);

        window_size1 = ImVec2(float(width), float(height) * 0.13f);
        window_size2 = ImVec2(float(width) * 0.1f, float(height) * 0.1f);

        buttons_size1 = ImVec2(float(width) / 4.f, float(height) / 7.f);

        font_scale1 = float(width) / 400.f;
        font_scale2 = float(width) / 800.f;

        text_position1 = float(width) * 0.45f;
    }
    else {
        window_position1 = window_size1 = window_size2 = buttons_size1 = ImVec2(1, 1);
        font_scale1 = font_scale2 = text_position1 = 1;
    }

    // Display loading screen
    if (display_loading) {
        if (game_manager->stage_timer.GetElapsedTime() < 6.f) {
            ImGui::Begin("Menu", pOpen, flags);
            ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetWindowSize(ImVec2(width, height), ImGuiCond_Always);
            ImGui::SetWindowFontScale(font_scale1);

            LoadMenuImage();
            ImGui::SetCursorPos(ImVec2(width * 0.45f, height * 0.02f));
            ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), "Pro Tip:");
            ImGui::SetCursorPosX(width * 0.08);
            ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), "Don't forget to spend Gold to improve your stats");

            ImGui::SetCursorPosY(height * 0.92f);
            ImGui::ProgressBar(game_manager->stage_timer.GetElapsedTime() / 6.f, ImVec2(width * 0.99, height * 0.05f));

            ImGui::End();
            return;
        }
        else if (game_manager->stage_timer.GetElapsedTime() < 9.1f) {
            ImGui::Begin("Menu", pOpen, flags);
            ImGui::SetWindowPos(window_position1, ImGuiCond_Always);
            ImGui::SetWindowSize(window_size2, ImGuiCond_Always);

            if ((6.1f < game_manager->stage_timer.GetElapsedTime()) && game_manager->stage_timer.GetElapsedTime() < 7.0f) {
                SetMenuImage("3.png");
                LoadMenuImage();
            }
            else if ((7.1f < game_manager->stage_timer.GetElapsedTime()) && (game_manager->stage_timer.GetElapsedTime() < 8.0f)) {
                SetMenuImage("2.png");
                LoadMenuImage();
            }
            else if ((8.1f < game_manager->stage_timer.GetElapsedTime()) && (game_manager->stage_timer.GetElapsedTime() < 9.0f)) {
                SetMenuImage("1.png");
                LoadMenuImage();
            }

            ImGui::End();

            // Play Music
            string stage_music;
            if (game_manager->sound_manager->playing_index != -game_manager->sound_manager->stage_index) {
                stage_music = "stage" + std::to_string(game_manager->sound_manager->stage_index) + ".mp3";
                game_manager->sound_manager->HandleMusic(stage_music);
                game_manager->sound_manager->playing_index = -game_manager->sound_manager->stage_index;
            }

            return;
        }
        else {
            SetMenuImage("stage.jpg");
            animate = true;
            display_loading = false;
            
            // Reset Timers
            game_manager->ResetTimers();
        }
    }

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale2);

    LoadMenuImage();

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(1.0, 0.5, 1.0, 1.0), "Stage Menu");

    Spacing(1);

    // Handle Stage
    gui_text = "Stage: " + std::to_string(game_manager->stats->selected_stage);
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.15f);

    // Handle Health
    gui_text = "Health: " + std::to_string(game_manager->stats->current_health);
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.3f);

    // Handle Score
    gui_text = "Score: " + std::to_string(game_manager->stats->current_score);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.45f);

    // Handle Gold
    gui_text = "Gold: " + std::to_string(game_manager->stats->gold);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());
    
    ImGui::SameLine(float(width) * 0.6f);

    // Handle Current Movement Speed 
    gui_text = "Movement Speed: " + std::to_string(game_manager->stats->current_movement_speed);
    ImGui::TextColored(ImVec4(0.0, 1.0, 1.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.8f);

    // Handle Stage Time
    float stage_time = game_manager->stage_timer.GetElapsedTime();
    gui_text = "Stage Time: " + game_manager->stage_timer.SecondsToGameTime(stage_time);
    ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Active Bonus
    int bonus_duration = game_manager->stats->bonuses_duration;
    int current_active_bonus_time = game_manager->stats->active_bonus_timer.GetElapsedTime();
    int bonus_duration_left = bonus_duration - current_active_bonus_time;
    string active_bonus = game_manager->stats->active_bonus;
    if (active_bonus == "None") {
        gui_text = "Active Bonus: " + game_manager->stats->active_bonus;
    }
    else {
        gui_text = "Active Bonus: " + game_manager->stats->active_bonus + " (" + to_string(bonus_duration_left) + "s)";
    }
    ImGui::TextColored(ImVec4(1.0, 0.0, 1.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.3f);

    // Handle Objective Score
    int objective_score = game_manager->stats->objective_score;
    int current_score = game_manager->stats->current_score;
    if (current_score < objective_score) {
        gui_text = "Objective Score: " + std::to_string(objective_score);
    }
    else {
        gui_text = "Objective: Go to the EXIT!";
    }
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), gui_text.c_str());

    ImGui::SameLine(float(width) * 0.6f);

    // Handle View
    ImGui::Text("Camera List: ");
    for (int i = 0; i < camera_list.size(); i++) {
        ImGui::SameLine();
        bool selected_camera = camera_list[i] == camera;
        if (selected_camera) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        if (ImGui::Button(std::to_string(i + 1).c_str())) {
            SetCamera(i);
            camera_index = i;
        }
        if (selected_camera) {
            ImGui::PopStyleColor();
        }
    }

    if (game_manager->stats->current_health == 0) {
        game_manager->sound_manager->StopMusic();
        SetAnimate(false);
        game_manager->sound_manager->HandleSound("obstacle_object.mp3");
        game_manager->sound_manager->HandleSound("game_over.mp3");
        game_manager->stats->total_deaths++;
        menu_index = StageFailedMenu;
    }

    if (game_manager->stats->stage_completed) {
        game_manager->sound_manager->StopMusic();
        SetAnimate(false);
        game_manager->sound_manager->HandleSound("stage_complete.mp3");
        menu_index = StageCompletedMenu;
    }

    ImGui::End();
}

void BasicScene::PauseMenuHandler() {
    string stage_music;
    if (game_manager->sound_manager->playing_index != -game_manager->sound_manager->stage_index) {
        stage_music = "stage" + std::to_string(game_manager->sound_manager->stage_index) + ".mp3";
        game_manager->sound_manager->HandleMusic(stage_music);
        game_manager->sound_manager->playing_index = -game_manager->sound_manager->stage_index;

        SetMenuImage("stage.jpg");
    }

    // Set sizes
    ImVec2 window_position1, window_size1, buttons_size1;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_position1 = ImVec2(float(width) * 0.39f, float(height) * 0.3f);

        window_size1 = ImVec2(float(width) * 0.22f, float(height) * 0.45f);

        buttons_size1 = ImVec2(float(width) / 6.f, float(height) / 16.f);

        font_scale1 = float(width) / 800.f;

        text_position1 = float(width) * 0.06f;
        text_position2 = float(width) * 0.03f;
    }
    else {
        window_position1 = window_size1, buttons_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(window_position1, ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(1.0, 0.5, 0.5, 1.0), "Pause Menu");

    Spacing(5);

    // Handle Stage
    ImGui::SetCursorPosX(text_position2);
    int current_stage = game_manager->stats->selected_stage;
    gui_text = "Stage: " + std::to_string(current_stage);
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Health
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Health: " + std::to_string(game_manager->stats->current_health);
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Score
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Score: " + std::to_string(game_manager->stats->current_score);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Continue", buttons_size1)) {
        menu_index = StageMenu;
        
        game_manager->stage_timer.StartTimer();
        animate = true;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Setting", buttons_size1)) {
        last_menu_index = PauseMenu;
        menu_index = SettingsMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back To Main Menu", buttons_size1)) {
        next_menu_index = MainMenu;
        menu_index = NewHighScoreMenu;
    }

    ImGui::End();
}

void BasicScene::StageCompletedMenuHandler() {
    // Menu with no sound
    game_manager->sound_manager->StopMusic();

    // Set sizes
    ImVec2 window_position1, window_size1, buttons_size1;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_position1 = ImVec2(float(width) * 0.39f, float(height) * 0.3f);

        window_size1 = ImVec2(float(width) * 0.22f, float(height) * 0.45f);

        buttons_size1 = ImVec2(float(width) / 6.f, float(height) / 16.f);

        font_scale1 = float(width) / 800.f;

        text_position1 = float(width) * 0.045f;
        text_position2 = float(width) * 0.03f;
    }
    else {
        window_position1 = window_size1, buttons_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    game_manager->stats->stage_completed = false;

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(window_position1, ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.5, 1.0), "Stage Completed");

    Spacing(5);

    // Handle Stage
    ImGui::SetCursorPosX(text_position2);
    int current_stage = game_manager->stats->selected_stage;
    gui_text = "Stage: " + std::to_string(current_stage);
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Score
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Score: " + std::to_string(game_manager->stats->current_score);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Gold
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Gold: " + std::to_string(game_manager->stats->gold);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (current_stage == 3) {
        if (ImGui::Button("End Game", buttons_size1)) {
            next_menu_index = CreditsMenu;
            menu_index = NewHighScoreMenu;
        }
    }
    else {
        if (ImGui::Button("Continue", buttons_size1)) {
            current_stage++;
            menu_index = StageMenu;
            game_manager->sound_manager->stage_index = current_stage;

            game_manager->LoadStage(current_stage, false);
            SetMenuImage("loading.jpg");
            display_loading = true;
        }
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Shop", buttons_size1)) {
        last_menu_index = StageCompletedMenu;
        menu_index = ShopMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back To Main Menu", buttons_size1)) {
        next_menu_index = MainMenu;
        menu_index = NewHighScoreMenu;
    }

    ImGui::End();
}

void BasicScene::StageFailedMenuHandler() {
    // Menu with no sound
    game_manager->sound_manager->StopMusic();

    // Set sizes
    ImVec2 window_position1, window_size1, buttons_size1;
    float font_scale1, text_position1, text_position2;

    if (width != 0 && height != 0) {
        window_position1 = ImVec2(float(width) * 0.39f, float(height) * 0.3f);

        window_size1 = ImVec2(float(width) * 0.22f, float(height) * 0.45f);

        buttons_size1 = ImVec2(float(width) / 6.f, float(height) / 16.f);

        font_scale1 = float(width) / 800.f;

        text_position1 = float(width) * 0.06f;
        text_position2 = float(width) * 0.03f;
    }
    else {
        window_position1 = window_size1, buttons_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = text_position2 = 1;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(window_position1, ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(1.0, 0.5, 0.0, 1.0), "Stage Failed");

    Spacing(5);

    // Handle Stage
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Stage: " + std::to_string(game_manager->stats->selected_stage);
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Score
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Score: " + std::to_string(game_manager->stats->current_score);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(1);

    // Handle Gold
    ImGui::SetCursorPosX(text_position2);
    gui_text = "Gold: " + std::to_string(game_manager->stats->gold);
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Retry", buttons_size1)) {
        next_menu_index = StageSelectionMenu;
        menu_index = NewHighScoreMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Shop", buttons_size1)) {
        last_menu_index = StageFailedMenu;
        menu_index = ShopMenu;
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position2);
    if (ImGui::Button("Back To Main Menu", buttons_size1)) {
        next_menu_index = MainMenu;
        menu_index = NewHighScoreMenu;
    }

    ImGui::End();
}

void BasicScene::NewHighScoreMenuHandler() {
    if (game_manager->sound_manager->playing_index != HallOfFameMenu) {
        game_manager->sound_manager->HandleMusic("hall_of_fame.mp3");
        game_manager->sound_manager->playing_index = HallOfFameMenu;
    }

    // Set sizes
    ImVec2 window_position1, window_size1, buttons_size1;
    float font_scale1, text_position1;

    if (width != 0 && height != 0) {
        window_position1 = ImVec2(float(width) * 0.3f, float(height) * 0.25f);

        window_size1 = ImVec2(float(width) * 0.4f, float(height) * 0.5f);

        buttons_size1 = ImVec2(float(width) / 3.f, float(height) / 13.f);

        font_scale1 = float(width) / 400.f;

        text_position1 = float(width) * 0.025f;
    }
    else {
        window_position1 = window_size1, buttons_size1 = ImVec2(1, 1);
        font_scale1 = text_position1 = 1;
    }

    int position = game_manager->leaderboard->CalculateLeaderboardPosition(game_manager->stats->current_score);
    // No new high score found
    if (position == -1) {
        menu_index = next_menu_index;

        game_manager->UnloadStage();
        animate = false;
        return;
    }

    int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    bool* pOpen = nullptr;
    string gui_text;

    ImGui::Begin("Menu", pOpen, flags);
    ImGui::SetWindowPos(window_position1, ImGuiCond_Always);
    ImGui::SetWindowSize(window_size1, ImGuiCond_Always);
    ImGui::SetWindowFontScale(font_scale1);

    ImGui::SetCursorPosX(text_position1);
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "N");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "e");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), "w");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), " H");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "i");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), "g");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "h");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), " S");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), "c");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "o");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "r");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0, 0.0, 1.0, 1.0), "e");

    Spacing(5);

    ImGui::SetCursorPosX(text_position1);
    ImGui::Text("Code Name: ");

    Spacing(5);

    ImGui::SetCursorPosX(text_position1);
    gui_text = to_string(position + 1) + ".";
    if (position == 0) {
        ImGui::TextColored(ImVec4(201.f / 176.f, 149.f / 255.f, 55.f / 255.f, 1.0), gui_text.c_str());
    }
    else if (position == 1) {
        ImGui::TextColored(ImVec4(180.f / 255.f, 180.f / 255.f, 180.f / 255.f, 1.0), gui_text.c_str());
    }
    else if (position == 2) {
        ImGui::TextColored(ImVec4(173.f / 255.f, 138.f / 255.f, 86.f / 255.f, 1.0), gui_text.c_str());
    }
    else {
        ImGui::Text(gui_text.c_str());
    }

    ImGui::SameLine();
    static char name[4] = "";
    ImGui::InputTextMultiline("", name, IM_ARRAYSIZE(name), ImVec2((width / 15), (height / 16)));

    ImGui::SameLine();
    gui_text = "-Score: " + to_string(game_manager->stats->current_score);
    ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), gui_text.c_str());

    Spacing(5);

    ImGui::SetCursorPosX(text_position1);
    if (ImGui::Button("Apply", buttons_size1)) {
        if (string(name).length() == 3) {
            NewHighScoreMenu_InvalidParameter = false;
            game_manager->leaderboard->AddScoreToLeaderboard(position, name, game_manager->stats->current_score);
            menu_index = next_menu_index;

            game_manager->UnloadStage();
            animate = false;
        }
        else {
            NewHighScoreMenu_InvalidParameter = true;
        }
    }

    if (NewHighScoreMenu_InvalidParameter) {
        ImGui::SetCursorPosX(text_position1);
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Invalid Code Name!");
    }

    Spacing(5);

    ImGui::SetCursorPosX(text_position1);
    if (ImGui::Button("Back To Main Menu", buttons_size1)) {
        menu_index = MainMenu;

        game_manager->UnloadStage();
        animate = false;
    }

    ImGui::End();
}

void BasicScene::Spacing(int number_of_spacing) {
    for (int i = 0; i < number_of_spacing; i++) {
        ImGui::Spacing();
    }
}

bool BasicScene::ProgramHandler(const Program& program) {
    bool default_behavior = true;

    if (program.name == "snake head program") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", snake_color.x(), snake_color.y(), snake_color.z(), 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, -15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "snake body program") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", snake_color.x(), snake_color.y(), snake_color.z(), 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "health") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 1.0f, 0.0f, 0.0f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "score") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 0.0f, 1.0f, 0.0f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "gold") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 1.0f, 1.0f, 0.0f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "bonus") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 1.0f, 0.0, 1.0f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "obstacle") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 0.3f, 0.3f, 0.3f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, 15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "exit") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 0.0f, 0.0f, 0.0f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, -15.0f, 0.0, 1.0f);

        default_behavior = false;
    }
    if (program.name == "bezier") {
        program.SetUniform4f("lightColor", 0.7f, 0.7f, 0.7f, 0.5f);
        program.SetUniform4f("Kai", 0.2f, 0.2f, 0.2f, 1.0f);
        program.SetUniform4f("Kdi", 0.5f, 0.5f, 0.5f, 1.0f);
        program.SetUniform1f("specular_exponent", 5.0f);
        program.SetUniform4f("light_position", 0.0, -15.0f, 0.0, 1.0f);

        default_behavior = false;
    }

    return default_behavior;
}

void BasicScene::SetMenuImage(string image_name) {
    string image_path = "images/" + image_name;
    background_image = stbi_load(image_path.c_str(), &image_width, &image_height, &channels, STBI_rgb_alpha);
    if (background_image == NULL) {
        // Error handling if the image couldn't be loaded
        std::cout << "image not loaded! " << std::endl;
    }
    else {
        glGenTextures(1, &background_image_texture);
        glBindTexture(GL_TEXTURE_2D, background_image_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, background_image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        stbi_image_free(background_image);
    }
}

void BasicScene::LoadMenuImage() {
    // Set Image
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 window_position = ImGui::GetWindowPos() - ImVec2(5,5);
    ImVec2 image_position = window_position + ImGui::GetCursorPos();
    ImVec2 image_size = window_size;

    ImGui::GetWindowDrawList()->AddImage(
        (void*)(intptr_t)background_image_texture,
        image_position,
        image_position + image_size,
        ImVec2(0, 0),
        ImVec2(1, 1),
        ImColor(255, 255, 255, 255)
    );
}
