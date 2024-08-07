#include <stdio.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

static TextureCubemap TextureToCubemap(Shader shader, Texture2D panorama, int size, int format)
{
    TextureCubemap cubemap = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    cubemap.id = rlLoadTextureCubemap(0, size, format);

    unsigned int fbo = rlLoadFramebuffer();
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

    if (rlFramebufferComplete(fbo)) 
    {
        TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
    }

    rlEnableShader(shader.id);

    Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    Matrix fboViews[6] = {
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f,  1.0f }),
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f,  0.0f, -1.0f }),
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt({ 0.0f, 0.0f, 0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f })
    };

    rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions
    
    rlActiveTextureSlot(0);
    rlEnableTexture(panorama.id);

    for (int i = 0; i < 6; i++)
    {
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
        
        rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
        rlEnableFramebuffer(fbo);

        rlClearScreenBuffers();
        rlLoadDrawCube();
    }

    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();

    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = format;

    return cubemap;
}

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetTraceLogLevel(LOG_DEBUG);
    // TODO: MSAA Fallback in rcore_desktop_glfw.c:1331
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE);

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
    Shader skyboxShader = LoadShader("../shader/skybox_vert.glsl", "../shader/skybox_frag.glsl");
    int uniform = MATERIAL_MAP_CUBEMAP;
    SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "environmentMap"), &uniform, SHADER_UNIFORM_INT);
    skybox.materials[0].shader = skyboxShader;

    Shader cubemapShader = LoadShader("../shader/cubemap_vert.glsl", "../shader/cubemap_frag.glsl");
    uniform = 0;
    SetShaderValue(cubemapShader, GetShaderLocation(cubemapShader, "equirectangularMap"), &uniform, SHADER_UNIFORM_INT);
    Texture2D skyboxTexture = LoadTexture("../assets/dresden_square_2k.hdr");
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = TextureToCubemap(cubemapShader, skyboxTexture, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    UnloadTexture(skyboxTexture);

    // Shadow setup
    Material shadowMaterial = LoadMaterialDefault();
    shadowMaterial.shader = LoadShader("../shader/shadow_vert.glsl", "../shader/shadow_frag.glsl");

    int shadowMapSize = 1024;
    Texture shadowMap = {};
    shadowMap.id = rlLoadTextureDepth(shadowMapSize, shadowMapSize, false);
    shadowMap.width = shadowMapSize;
    shadowMap.height = shadowMapSize;
    unsigned int shadowMapFramebuffer = rlLoadFramebuffer();
    rlFramebufferAttach(shadowMapFramebuffer, shadowMap.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

    if (rlFramebufferComplete(shadowMapFramebuffer)) 
    {
        TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", shadowMapFramebuffer);
    }

    // Matrix projection = MatrixOrtho(-shadowMapSize / 2, shadowMapSize / 2, -shadowMapSize / 2, shadowMapSize / 2, 1, 100);


    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);   

        // Start shadow map
        rlViewport(0, 0, shadowMapSize, shadowMapSize);
        rlEnableFramebuffer(shadowMapFramebuffer);
        rlClearScreenBuffers();

        // rlSetUniformMatrix(shadowMaterial.shader.locs[SHADER_LOC_MATRIX_PROJECTION], projection);
        rlMatrixMode(RL_PROJECTION);
        rlPushMatrix();
        rlLoadIdentity();
        rlOrtho(-shadowMapSize / 2, shadowMapSize / 2, -shadowMapSize / 2, shadowMapSize / 2, 1, 100);
        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();

        DrawModelMaterial(model, position, 1.0f, shadowMaterial, WHITE);

        rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
        rlUnloadFramebuffer(shadowMapFramebuffer);

        // End shadowmap

        ClearBackground(RAYWHITE);

        BeginDrawing();
        BeginMode3D(camera);

        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        DrawModel(skybox, {0, 0, 0}, 1.0f, WHITE);
        rlEnableBackfaceCulling();
        rlEnableDepthMask();

        DrawModel(model, position, 1.0f, WHITE);

        DrawModel(model, position, 1.0f, WHITE);
        DrawGrid(20, 10.0f);

        EndMode3D();

        DrawTexture(shadowMap, 0, 0, WHITE);

        DrawFPS(10, 10);

        EndDrawing();
    }
    
    CloseWindow();
}
