#include <stdio.h>

#include "raylib.h"

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetTraceLogLevel(LOG_DEBUG);
    // TODO: MSAA Fallback in rcore_desktop_glfw.c:1331
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "raylib [models] example - models loading");
    DisableCursor();

    Camera camera = { 0 };
    camera.position = { 50.0f, 50.0f, 50.0f };
    camera.target = { 0.0f, 10.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;    

    Model model = LoadModel("../assets/Sponza/glTF/Sponza.gltf");
    //for (int i = 0; i < model.materialCount; ++i) 
    //{
    //    GenTextureMipmaps(&model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture);
    //}

    Vector3 position = { 0.0f, 0.0f, 0.0f };

    // Skybox setup
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);
    skybox.materials[0].shader = LoadShader("../shader/skybox_vert.glsl", "../shader/skybox_frag.glsl");

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);   

        ClearBackground(RAYWHITE);

        BeginDrawing();
        BeginMode3D(camera);

        DrawModel(model, position, 1.0f, WHITE);
        DrawGrid(20, 10.0f);
        EndMode3D();

        DrawFPS(10, 10);

        EndDrawing();
    }
    
    CloseWindow();
}
