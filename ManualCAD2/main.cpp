#include "glApplication.h"

using namespace ManualCAD;

int main(int, char**)
{
    //Raycaster raycaster;
    Renderer renderer;
    GlApplication app(1280, 720, "ManualCAD 2", ImVec4(0.27f, 0.33f, 0.36f, 1.00f), renderer);

    //app.add_window(make_window<ImGuiDemoWindow>());
    app.add_internal_windows();

    app.get_io().Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);

    app.maximize();

    app.run();

    app.dispose();

    return 0;
}