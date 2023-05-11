#pragma once
#include "AutoMorphingModel.h"
#include "Scene.h"

#include <memory>
#include <utility>
#include <vector>

#include "SceneWithImGui.h"
#include "GameManager.h"

#include <imgui.h>

class BasicScene : public cg3d::SceneWithImGui
{
public:
    BasicScene(std::string name, cg3d::Display* display);
    void Init(float fov, int width, int height, float near, float far);
    void Update(const cg3d::Program& program, const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, const Eigen::Matrix4f& model) override;
    void MouseCallback(cg3d::Viewport* viewport, int x, int y, int button, int action, int mods, int buttonState[]) override;
    void ScrollCallback(cg3d::Viewport* viewport, int x, int y, int xoffset, int yoffset, bool dragging, int buttonState[]) override;
    void CursorPosCallback(cg3d::Viewport* viewport, int x, int y, bool dragging, int* buttonState)  override;
    void KeyCallback(cg3d::Viewport* viewport, int x, int y, int key, int scancode, int action, int mods) override;

    // SceneWithImGui expend
    void SetCamera(int index);
    void AddViewportCallback(cg3d::Viewport* _viewport) override;
    void ViewportSizeCallback(cg3d::Viewport* _viewport) override;
    

    // Camera View Management
    void SwitchView(bool next);


    // Game Manager
    GameManager* game_manager;


    // Menu Managmenet
    void MenuManager();
    enum MenuMapping { 
        LoginMenu, 
        MainMenu, 
        StageSelectionMenu, 
        ShopMenu, 
        StatsMenu, 
        HallOfFameMenu, 
        SettingsMenu,
        CreditsMenu, 
        StageMenu,
        PauseMenu,
        StageCompletedMenu, 
        StageFailedMenu, 
        NewHighScoreMenu
    };

    void LoginMenuHandler();
    void MainMenuHandler();
    void ShopMenuHandler();
    void StatsMenuHandler();
    void StageSelectionMenuHandler();
    void HallOfFameMenuHandler();
    void SettingsMenuHandler();
    void CreditsMenuHandler();
    void StageMenuHandler();
    void PauseMenuHandler();
    void StageCompletedMenuHandler();
    void StageFailedMenuHandler();
    void NewHighScoreMenuHandler();
    void Spacing(int number_of_spacing);

    // Programs Handler
    bool ProgramHandler(const Program& program);

    void SetMenuImage(string image_name);
    void LoadMenuImage();

private:
    // SceneWithImGui expend
    void BuildImGui() override;
    cg3d::Viewport* viewport = nullptr;


    // Scene root
    std::shared_ptr<Movable> root;


    // Camera list parameters
    std::vector<std::shared_ptr<cg3d::Camera>> camera_list{ 4 };
    int camera_index = 0;
    int number_of_cameras = 0;


    // Menu parameters
    int width = 1;
    int height = 1;

    int menu_index = LoginMenu;
    int last_menu_index = MainMenu; // For ShopMenu
    int next_menu_index = MainMenu; // For NewHighScoreMenu

    bool display_keys = false;
    bool payed_credits = false;
    bool display_new_game = true;
    bool display_loading = true;

    // Invalid Checks
    bool Login_InvalidName = false;
    bool Login_InvalidSaveData = false;
    bool ShopMenu_InvalidGold1 = false;
    bool ShopMenu_InvalidGold2 = false;
    bool ShopMenu_InvalidGold3 = false;
    bool ShopMenu_InvalidGold4 = false;
    bool ShopMenu_InvalidGold5 = false;
    bool NewHighScoreMenu_InvalidParameter = false;


    // Snake Color
    Eigen::Vector3f snake_color = Eigen::Vector3f(0.5f, 0.25f, 0.0f);

    // Menu Images
    unsigned char* background_image;
    GLuint background_image_texture;
    int image_width;
    int image_height;
    int screen_width;
    int screen_height;
    int channels;

    // Debug mode
    int debug_parameter = 0;
};
