// CS370 Final Project
// Fall 2023

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/tangentspace.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, TexCube, Cylinder, Cone, Mug, Frame, Mirror, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, TangBuffer, BiTangBuffer, NumObjBuffers};
enum Color_Buffer_IDs {WhiteCube, Switch, Walls, BlackMat, BlackCone, WoodFrame, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {White, OffWhite, Blue, StandingLight, WoodLining, Glass, Liquid, Tin};
enum Textures {Blank, Wood, Carpet, Roof, Door, Widow, CarpetNorm, RoofNorm, DoorNorm, WoodNorm, ShadowTex, MirrorTex, NumTextures};
enum LightNames {WhitePointLight, WhiteSpotLight};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];
GLuint ShadowBuffer;

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint tangCoords = 3;
GLint bitangCoords = 3;
GLint colCoords = 4;

// Model files
const char * cubeFile = "../models/unitcube.obj";
const char * cylinderFile = "../models/cylinder.obj";
const char * coneFile = "../models/cone.obj";
const char * mugFile = "../models/mug.obj";
const char * planeFile = "../models/plane.obj";

// Texture files
const char * blankFile = "../textures/blank.png";
const char * woodFile = "../textures/wood.png";
const char * carpetFile = "../textures/carpet.jpg";
const char * roofFile = "../textures/roof.jpg";
const char * doorFile = "../textures/door.jpg";
const char * windowFile = "../textures/landscape.jpg";
const char * carpetNormFile = "../textures/CarpetMap.png";
const char * roofNormFile = "../textures/RoofMap.png";
const char * doorNormFile = "../textures/DoorMap.png";
const char * woodNormFile = "../textures/FloorMap.png";

// Camera
vec3 eye = {-3.0f, 2.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
vec3 dir = {1.0f, 1.0f, 1.0f};
vec3 mirror_eye = {0.0f, 2.0f, 5.1f};
vec3 mirror_center = {0.0f, 0.0f, 0.0f};
vec3 mirror_up = {0.0f, 1.0f, 0.0f};
GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;
GLfloat camera_angle = 0.0f;
GLfloat stepSize = 0.5f;
GLboolean spin = true;
GLboolean blinds = false;
GLfloat swtich1_ang = 45.0f;
GLfloat swtich2_ang = 45.0f;
GLfloat swtich3_ang = 45.0f;
GLfloat blade_ang = 0.0f;
GLfloat blade_dps = 2.0f;
GLfloat blinds_ang = 0.0f;
GLfloat blinds_dps = 360.0f;
GLfloat spin_dir = 1.0f;
GLdouble elTime = 0.0;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Light shader program with shadows reference
GLuint phong_shadow_program;
GLuint phong_shadow_vPos;
GLuint phong_shadow_vNorm;
GLuint phong_shadow_proj_mat_loc;
GLuint phong_shadow_camera_mat_loc;
GLuint phong_shadow_norm_mat_loc;
GLuint phong_shadow_model_mat_loc;
GLuint phong_shadow_shad_proj_mat_loc;
GLuint phong_shadow_shad_cam_mat_loc;
GLuint phong_shadow_lights_block_idx;
GLuint phong_shadow_materials_block_idx;
GLuint phong_shadow_material_loc;
GLuint phong_shadow_num_lights_loc;
GLuint phong_shadow_light_on_loc;
GLuint phong_shadow_eye_loc;
const char *phong_shadow_vertex_shader = "../phongShadow.vert";
const char *phong_shadow_frag_shader = "../phongShadow.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Shadow shader program reference
GLuint shadow_program;
GLuint shadow_vPos;
GLuint shadow_proj_mat_loc;
GLuint shadow_camera_mat_loc;
GLuint shadow_model_mat_loc;
const char *shadow_vertex_shader = "../shadow.vert";
const char *shadow_frag_shader = "../shadow.frag";

// Bumpmapping shader program reference
GLuint bump_program;
GLuint bump_proj_mat_loc;
GLuint bump_camera_mat_loc;
GLuint bump_norm_mat_loc;
GLuint bump_model_mat_loc;
GLuint bump_vPos;
GLuint bump_vNorm;
GLuint bump_vTex;
GLuint bump_vTang;
GLuint bump_vBiTang;
GLuint bump_lights_block_idx;
GLuint bump_num_lights_loc;
GLuint bump_light_on_loc;
GLuint bump_eye_loc;
GLuint bump_base_loc;
GLuint bump_norm_loc;
const char *bump_vertex_shader = "../bumpTex.vert";
const char *bump_frag_shader = "../bumpTex.frag";

// BumpShadow shader program reference
GLuint bumpShadow_program;
GLuint bumpShadow_proj_mat_loc;
GLuint bumpShadow_camera_mat_loc;
GLuint bumpShadow_norm_mat_loc;
GLuint bumpShadow_model_mat_loc;
GLuint bumpShadow_vPos;
GLuint bumpShadow_vNorm;
GLuint bumpShadow_vTex;
GLuint bumpShadow_vTang;
GLuint bumpShadow_vBiTang;
GLuint bumpShadow_lights_block_idx;
GLuint bumpShadow_num_lights_loc;
GLuint bumpShadow_light_on_loc;
GLuint bumpShadow_eye_loc;
GLuint bumpShadow_base_loc;
GLuint bumpShadow_norm_loc;
GLuint bumpShadow_shadow_loc;
GLuint bumpShadow_shad_proj_mat_loc;
GLuint bumpShadow_shad_cam_mat_loc;
GLuint bumpShadow_materials_block_idx;
GLuint bumpShadow_material_loc;
const char *bumpShadow_vertex_shader = "../bumpShadow.vert";
const char *bumpShadow_frag_shader = "../bumpShadow.frag";

// Debug shadow program reference
GLuint debug_program;
const char *debug_shadow_vertex_shader = "../debugShadow.vert";
const char *debug_shadow_frag_shader = "../debugShadow.frag";

// Shadow flag
GLuint shadow = false;

// Mirror flag
GLboolean mirror = false;

// Generic shader variables references
GLuint vPos;
GLuint vNorm;
GLuint model_mat_loc;

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;
mat4 shadow_proj_matrix;
mat4 shadow_camera_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();
void create_shadows( );
void create_mirror( );
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_mirror(GLuint m_textid);
void build_frame(GLuint obj);
void build_textures();
void build_texture_cube(GLuint obj);
void build_shadows( );
void load_model(const char * filename, GLuint obj);
void load_texture(const char * filename, GLuint texID, GLint magFilter, GLint minFilter, GLint sWrap, GLint tWrap, bool mipMap, bool invert);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void draw_bump_object(GLuint obj, GLuint base_texture, GLuint normal_map);
void draw_bump_shadow_object(GLuint obj, GLuint base_texture, GLuint normal_map);
void draw_frame(GLuint obj);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void renderQuad();

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load light shader with shadows
    ShaderInfo phong_shadow_shaders[] = { {GL_VERTEX_SHADER, phong_shadow_vertex_shader},{GL_FRAGMENT_SHADER, phong_shadow_frag_shader},{GL_NONE, NULL} };
    phong_shadow_program = LoadShaders(phong_shadow_shaders);
    phong_shadow_vPos = glGetAttribLocation(phong_shadow_program, "vPosition");
    phong_shadow_vNorm = glGetAttribLocation(phong_shadow_program, "vNormal");
    phong_shadow_camera_mat_loc = glGetUniformLocation(phong_shadow_program, "camera_matrix");
    phong_shadow_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "proj_matrix");
    phong_shadow_norm_mat_loc = glGetUniformLocation(phong_shadow_program, "normal_matrix");
    phong_shadow_model_mat_loc = glGetUniformLocation(phong_shadow_program, "model_matrix");
    phong_shadow_shad_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "light_proj_matrix");
    phong_shadow_shad_cam_mat_loc = glGetUniformLocation(phong_shadow_program, "light_cam_matrix");
    phong_shadow_lights_block_idx = glGetUniformBlockIndex(phong_shadow_program, "LightBuffer");
    phong_shadow_materials_block_idx = glGetUniformBlockIndex(phong_shadow_program, "MaterialBuffer");
    phong_shadow_material_loc = glGetUniformLocation(phong_shadow_program, "Material");
    phong_shadow_num_lights_loc = glGetUniformLocation(phong_shadow_program, "NumLights");
    phong_shadow_light_on_loc = glGetUniformLocation(phong_shadow_program, "LightOn");
    phong_shadow_eye_loc = glGetUniformLocation(phong_shadow_program, "EyePosition");

    // Load shadow shader
    ShaderInfo shadow_shaders[] = { {GL_VERTEX_SHADER, shadow_vertex_shader},{GL_FRAGMENT_SHADER, shadow_frag_shader},{GL_NONE, NULL} };
    shadow_program = LoadShaders(shadow_shaders);
    shadow_vPos = glGetAttribLocation(shadow_program, "vPosition");
    shadow_proj_mat_loc = glGetUniformLocation(shadow_program, "light_proj_matrix");
    shadow_camera_mat_loc = glGetUniformLocation(shadow_program, "light_cam_matrix");
    shadow_model_mat_loc = glGetUniformLocation(shadow_program, "model_matrix");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Load bump shader
    ShaderInfo bump_shaders[] = { {GL_VERTEX_SHADER, bump_vertex_shader},{GL_FRAGMENT_SHADER, bump_frag_shader},{GL_NONE, NULL} };
    bump_program = LoadShaders(bump_shaders);
    bump_vPos = glGetAttribLocation(bump_program, "vPosition");
    bump_vNorm = glGetAttribLocation(bump_program, "vNormal");
    bump_vTex = glGetAttribLocation(bump_program, "vTexCoord");
    bump_vTang = glGetAttribLocation(bump_program, "vTangent");
    bump_vBiTang = glGetAttribLocation(bump_program, "vBiTangent");
    bump_proj_mat_loc = glGetUniformLocation(bump_program, "proj_matrix");
    bump_camera_mat_loc = glGetUniformLocation(bump_program, "camera_matrix");
    bump_norm_mat_loc = glGetUniformLocation(bump_program, "normal_matrix");
    bump_model_mat_loc = glGetUniformLocation(bump_program, "model_matrix");
    bump_lights_block_idx = glGetUniformBlockIndex(bump_program, "LightBuffer");
    bump_num_lights_loc = glGetUniformLocation(bump_program, "NumLights");
    bump_light_on_loc = glGetUniformLocation(bump_program, "LightOn");
    bump_eye_loc = glGetUniformLocation(bump_program, "EyePosition");
    bump_base_loc = glGetUniformLocation(bump_program, "baseMap");
    bump_norm_loc = glGetUniformLocation(bump_program, "normalMap");

    ShaderInfo bumpShadow_shaders[] = { {GL_VERTEX_SHADER, bumpShadow_vertex_shader},{GL_FRAGMENT_SHADER, bumpShadow_frag_shader},{GL_NONE, NULL} };
    bumpShadow_program = LoadShaders(bumpShadow_shaders);
    bumpShadow_proj_mat_loc = glGetAttribLocation(bumpShadow_program, "proj_matrix");
    bumpShadow_camera_mat_loc = glGetAttribLocation(bumpShadow_program, "camera_matrix");
    bumpShadow_norm_mat_loc = glGetAttribLocation(bumpShadow_program, "normal_matrix");
    bumpShadow_model_mat_loc = glGetAttribLocation(bumpShadow_program, "model_matrix");
    bumpShadow_vPos = glGetAttribLocation(bumpShadow_program, "vPosition");
    bumpShadow_vNorm = glGetAttribLocation(bumpShadow_program, "vNorm");
    bumpShadow_vTex = glGetAttribLocation(bumpShadow_program, "vTexCoord");
    bumpShadow_vTang = glGetAttribLocation(bumpShadow_program, "vTangent");
    bumpShadow_vBiTang = glGetAttribLocation(bumpShadow_program, "vBiTangent");
    bumpShadow_lights_block_idx = glGetAttribLocation(bumpShadow_program, "LightBuffer");
    bumpShadow_num_lights_loc = glGetAttribLocation(bumpShadow_program, "NumLights");
    bumpShadow_light_on_loc = glGetAttribLocation(bumpShadow_program, "LightOn");
    bumpShadow_eye_loc = glGetAttribLocation(bumpShadow_program, "EyePosition");
    bumpShadow_base_loc = glGetAttribLocation(bumpShadow_program, "baseMap");
    bumpShadow_norm_loc = glGetAttribLocation(bumpShadow_program, "normalMap");
    bumpShadow_norm_loc = glGetAttribLocation(bumpShadow_program, "shadowMap");
    bumpShadow_shad_proj_mat_loc = glGetAttribLocation(bumpShadow_program, "light_proj_matrix");
    bumpShadow_shad_cam_mat_loc = glGetAttribLocation(bumpShadow_program, "light_cam_matrix");
    bumpShadow_materials_block_idx = glGetAttribLocation(bumpShadow_program, "MaterialBuffer");
    bumpShadow_material_loc = glGetAttribLocation(bumpShadow_program, "Material");

    // Load debug shadow shader
    ShaderInfo debug_shaders[] = { {GL_VERTEX_SHADER, debug_shadow_vertex_shader},{GL_FRAGMENT_SHADER, debug_shadow_frag_shader},{GL_NONE, NULL} };
    debug_program = LoadShaders(debug_shaders);

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();
    // Create shadow buffer
    build_shadows();
    // Create mirror texture
    build_mirror(MirrorTex);

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Alpha Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        glCullFace(GL_FRONT);
        create_shadows();
        glCullFace(GL_BACK);
        center[0] = eye[0] + cos(camera_angle);
        center[1] = eye[1];
        center[2] = eye[2] + sin(camera_angle);

        create_mirror();
    	// Draw graphics
//    	renderQuad();
        display();
        // Update other events like input handling
        glfwPollEvents();

        //animation
        GLdouble curTime = glfwGetTime();
        if (spin) {
            blade_ang += (curTime - elTime) * (blade_dps / 60.0) * 360.0f;
        }

        if (blinds) {
            blinds_ang += spin_dir * (curTime - elTime) * (blinds_dps);
            if (blinds_ang <= 0.0f || blinds_ang >= 55.0f) {
                blinds = false;
                spin_dir *= -1;
            }
        }
        elTime = curTime;

        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // DEFAULT ORTHOGRAPHIC PROJECTION
    proj_matrix = frustum(-0.1f*xratio, 0.1f*xratio, -0.1f*yratio, 0.1f*yratio, 0.1f, 20.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Set cube transformation matrix
    // floor
    scale_matrix = scale(11.0f, 0.5f, 11.0f);
	model_matrix = scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Carpet, CarpetNorm);

    //walls
    trans_matrix = translate(5.5f, 2.0f, 0.0f);
    scale_matrix = scale(0.5f, 4.0f, 11.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, Blue);
    trans_matrix = translate(-5.5f, 2.0f, 0.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, Blue);
    trans_matrix = translate(0.0f, 2.0f, 5.5f);
    scale_matrix = scale(11.0f, 4.0f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, Blue);
    trans_matrix = translate(0.0f, 2.0f, -5.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, Blue);

    // roof
    trans_matrix = translate(0.0f, 4.0f, 0.0f);
    scale_matrix = scale(11.0f, 0.5f, 11.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Roof, RoofNorm);

    //table
    trans_matrix = translate(0.0f, 1.5f, 0.0f);
    scale_matrix = scale(2.0f, 0.3f, 2.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(0.85f, 1.0f, 0.85f);
    scale_matrix = scale(0.3f, 1.0f, 0.3f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(-0.85f, 1.0f, 0.85f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(-0.85f, 1.0f, -0.85f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(0.85f, 1.0f, -0.85f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);

    //can
    trans_matrix = translate(0.5f, 1.9f, 0.5f);
    scale_matrix = scale(0.2f, 0.23f, 0.2f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cylinder, Tin);

    //draw chair
    trans_matrix = translate(0.9f, 1.0f, 0.0f);
    scale_matrix = scale(1.0f, 0.1f, 1.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(1.35f, 1.0f, 0.45f);
    scale_matrix = scale(0.1f, 2.0f, 0.1f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(1.35f, 1.0f, -0.45f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(0.45f, 0.7f, 0.45f);
    scale_matrix = scale(0.1f, 0.5f, 0.1f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(0.45f, 0.7f, -0.45f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);
    trans_matrix = translate(1.35f, 1.7f, 0.0f);
    scale_matrix = scale(0.0f, 0.6f, 1.0f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Wood, WoodNorm);

    //standing light
    trans_matrix = translate(3.0f, 1.0f, 3.0f);
    scale_matrix = scale(0.15f, 1.0f, 0.15f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cylinder, StandingLight);
    trans_matrix = translate(3.0f, 1.9f, 3.0f);
    rot_matrix = rotate(180.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.3f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    rot_matrix = rotate(90.0f, 1.0f, 0.0f, 1.0f);
    model_matrix *= rot_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cone, StandingLight);
    trans_matrix = translate(3.0f, 0.45f, 3.0f);
    scale_matrix = scale(0.4f, 0.1f, 0.4f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cone, StandingLight);

    //light switch
    trans_matrix = translate(-5.2f, 2.0f, -3.3f);
    scale_matrix = scale(0.2f, 1.0f, 1.7f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, White);
    trans_matrix = translate(-5.1f, 2.1f, -3.3f);
    rot_matrix = rotate(swtich1_ang, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.7f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, OffWhite);
    trans_matrix = translate(-5.1f, 2.1f, -2.7f);
    rot_matrix = rotate(swtich2_ang, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.7f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, OffWhite);
    trans_matrix = translate(-5.1f, 2.1f, -3.9f);
    rot_matrix = rotate(swtich3_ang, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.7f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, OffWhite);

    //door
    trans_matrix = translate(-5.1f, 1.5f, 0.0f);
    rot_matrix = rotate(180.0f, 1.0f, 0.0f, 0.0f);
    scale_matrix = scale(0.1f, 4.0f, 2.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_bump_object(TexCube, Door, DoorNorm);

    //window
    trans_matrix = translate(0.0f, 2.0f, -5.25f);
    rot_matrix = rotate(180.0f, 0.0f, 0.0f, 1.0f);
    scale_matrix = scale(2.0f, 2.0f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_tex_object(TexCube, Widow);
    trans_matrix = translate(1.0f, 2.0f, -5.25f);
    scale_matrix = scale(0.3f, 2.3f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, WoodLining);
    trans_matrix = translate(-1.0f, 2.0f, -5.25f);
    scale_matrix = scale(0.3f, 2.3f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, WoodLining);
    trans_matrix = translate(0.0f, 3.0f, -5.25f);
    scale_matrix = scale(2.0f, 0.3f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, WoodLining);
    trans_matrix = translate(0.0f, 1.0f, -5.25f);
    scale_matrix = scale(2.0f, 0.3f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cube, WoodLining);

    //blinds
    for (float i = 3.0f; i > 1.0f; i -= 0.1f) {
        trans_matrix = translate(0.0f, i, -5.1f);
        scale_matrix = scale(1.68f, 0.05f, 0.1f);
        rot_matrix = rotate(blinds_ang, 1.0f, 0.0f, 0.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        if (!shadow) {
            // Set normal matrix for phong shadow shader
            normal_matrix = model_matrix.inverse().transpose();
        }
        draw_mat_object(Cube, White);
    }

    //fan
    trans_matrix = translate(0.0f, 3.3f, 0.0f);
    scale_matrix = scale(0.15f, 0.1f, 0.15f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cylinder, StandingLight);
    trans_matrix = translate(0.0f, 3.1f, 0.0f);
    scale_matrix = scale(0.5f, 0.05f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cylinder, StandingLight);

    //fan blade
    for (int i = 0; i < 2; i++) {
        scale_matrix = scale(2.65f, 0.05f, 0.4f);
        rot_matrix = rotate((90.0f * i) + blade_ang, 0.0f, 1.0f, 0.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        if (!shadow) {
            // Set normal matrix for phong shadow shader
            normal_matrix = model_matrix.inverse().transpose();
        }
        draw_mat_object(Cube, WoodLining);
    }

    //mirror
    if (!mirror) {
        draw_frame(Frame);
        trans_matrix = translate(mirror_eye);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(1.5f, 1.0f, 1.5f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        draw_tex_object(Mirror, MirrorTex);
    }


    //drink
    trans_matrix = translate(0.0f, 1.6f, 0.0f);
    scale_matrix = scale(0.25f, 0.25f, 0.25f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    glDepthMask(GL_FALSE);
    draw_mat_object(Mug, Glass);
    trans_matrix = translate(0.0f, 1.9f, 0.0f);
    scale_matrix = scale(0.2f, 0.2f, 0.2f);
    model_matrix = trans_matrix*scale_matrix;
    if (!shadow) {
        // Set normal matrix for phong shadow shader
        normal_matrix = model_matrix.inverse().transpose();
    }
    draw_mat_object(Cylinder, Liquid);
    glDepthMask(GL_TRUE);
}

void create_shadows( ){
    shadow_proj_matrix = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);

    vec3 leye = {Lights[0].position[0], Lights[0].position[1], Lights[0].position[2]};
    vec3 ldir = {Lights[0].direction[0], Lights[0].direction[1], Lights[0].direction[2]};
    vec3 lup = {0.0f, 1.0f, 0.0f};
    vec3 lcenter = leye + ldir;
    shadow_camera_matrix = lookat(leye, lcenter, lup);

    // Change viewport to match shadow framebuffer size
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    shadow = true;
    render_scene();
    shadow = false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport
    glViewport(0, 0, ww, hh);
}

void create_mirror( ) {
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    proj_matrix = frustum(-0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 100.0f);

    camera_matrix = lookat(mirror_eye, mirror_center, mirror_up);

    // Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);
}

void draw_bump_object(GLuint obj, GLuint base_texture, GLuint normal_map){
    // Select shader program
    glUseProgram(bump_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(bump_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(bump_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(bump_program, bump_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

    // Set camera position
    glUniform3fv(bump_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(bump_num_lights_loc, numLights);
    glUniform1iv(bump_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(bump_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(bump_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Set base texture to texture unit 0 and make it active
    glUniform1i(bump_base_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    // Bind base texture (to unit 0)
    glBindTexture(GL_TEXTURE_2D, TextureIDs[base_texture]);

    // Set normal map texture to texture unit 1 and make it active
    glUniform1i(bump_norm_loc, 1);
    glActiveTexture(GL_TEXTURE1);
    // Bind normal map texture (to unit 1)
    glBindTexture(GL_TEXTURE_2D, TextureIDs[normal_map]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(bump_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(bump_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vNorm);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(bump_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vTex);

    // Bind tangent object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glVertexAttribPointer(bump_vTang, tangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vTang);

    // Bind bitangent object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glVertexAttribPointer(bump_vBiTang, bitangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vBiTang);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_bump_shadow_object(GLuint obj, GLuint base_texture, GLuint normal_map){
    if (shadow) {
        // Use shadow shader
        glUseProgram(shadow_program);
        // Pass shadow projection and camera matrices to shader
        glUniformMatrix4fv(shadow_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(shadow_camera_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        // Set object attributes to shadow shader
        vPos = shadow_vPos;
        model_mat_loc = shadow_model_mat_loc;
    } else {
        // Select shader program
        glUseProgram(bumpShadow_program);

        // Pass projection and camera matrices to shader
        glUniformMatrix4fv(bumpShadow_proj_mat_loc, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(bumpShadow_camera_mat_loc, 1, GL_FALSE, camera_matrix);

        // Bind lights
        glUniformBlockBinding(bumpShadow_program, bump_lights_block_idx, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

        // Set camera position
        glUniform3fv(bumpShadow_eye_loc, 1, eye);

        // Set num lights and lightOn
        glUniform1i(bumpShadow_num_lights_loc, numLights);
        glUniform1iv(bumpShadow_light_on_loc, numLights, lightOn);

        // Pass model matrix and normal matrix to shader
        glUniformMatrix4fv(bumpShadow_norm_mat_loc, 1, GL_FALSE, normal_matrix);

        // Set base texture to texture unit 0 and make it active
        glUniform1i(bumpShadow_base_loc, 0);
        glActiveTexture(GL_TEXTURE0);
        // Bind base texture (to unit 0)
        glBindTexture(GL_TEXTURE_2D, TextureIDs[base_texture]);

        // Set normal map texture to texture unit 1 and make it active
        glUniform1i(bumpShadow_norm_loc, 1);
        glActiveTexture(GL_TEXTURE1);
        // Bind normal map texture (to unit 1)
        glBindTexture(GL_TEXTURE_2D, TextureIDs[normal_map]);

        glUniform1i(bumpShadow_shadow_loc, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);

        glUniformMatrix4fv(bumpShadow_shad_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(bumpShadow_shad_cam_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        // Set object attributes for phong shadow shader
        vPos = bumpShadow_vPos;
        model_mat_loc = bumpShadow_model_mat_loc;
    }

    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(vPos);

    if (!shadow) {
        // Bind normal object buffer and set attributes
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
        glVertexAttribPointer(bumpShadow_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(bumpShadow_vNorm);

        // Bind texture object buffer and set attributes
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
        glVertexAttribPointer(bumpShadow_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(bumpShadow_vTex);

        // Bind tangent object buffer and set attributes
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
        glVertexAttribPointer(bumpShadow_vTang, tangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(bumpShadow_vTang);

        // Bind bitangent object buffer and set attributes
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
        glVertexAttribPointer(bumpShadow_vBiTang, bitangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(bumpShadow_vBiTang);
    }

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_frame(GLuint obj){
    // Draw frame using lines at mirror location
    glUseProgram(lighting_program);
    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));
    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));
    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);
    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Set frame transformation matrix
    mat4 trans_matrix = translate(mirror_eye);
    mat4 rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    mat4 scale_matrix = scale(1.5f, 1.0f, 1.5f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);
    glUniform1i(lighting_material_loc, White);

    // Draw object using line loop
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);
    glDrawArrays(GL_LINE_LOOP, 0, numVertices[obj]);

}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_model(cubeFile, Cube);
    load_model(cylinderFile, Cylinder);
    load_model(coneFile, Cone);
    load_model(mugFile, Mug);
    load_model(planeFile, Mirror);

    build_texture_cube(TexCube);
    build_frame(Frame);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    // Define cube vertex colors (red)

}

void build_materials( ) {
    // Add materials to Materials vector
    MaterialProperties white = {
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //ambient
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //diffuse
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties offWhite = {
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //ambient
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //diffuse
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties blue = {
            vec4(0.0f, 0.0f, 0.5f, 1.0f), //ambient
            vec4(0.0f, 0.0f, 0.8f, 1.0f), //diffuse
            vec4(0.0f, 0.0f, 0.81f, 1.0f), //specular
            50.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties black = {
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.1f, 0.1f, 0.1f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 1.0f), //specular
            50.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties wood = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.38f, 0.212f, 0.098f, 1.0f), //diffuse
            vec4(0.78f, 0.64f, 0.1f, 1.0f), //specular
            1.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties glass = {
            vec4(0.5f, 0.5f, 0.5f, 0.3f), //ambient
            vec4(0.5f, 0.5f, 0.5f, 0.3f), //diffuse
            vec4(0.5f, 0.5f, 0.5f, 0.3f), //specular
            50.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties liquid = {
            vec4(0.0f, 1.0f, 0.0f, 0.8f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 0.8f), //diffuse
            vec4(0.0f, 1.0f, 0.0f, 0.8f), //specular
            30.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties tin = {
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //ambient
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //diffuse
            vec4(0.7f, 0.7f, 0.7f, 1.0f), //specular
            78.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    Materials.push_back(white);
    Materials.push_back(offWhite);
    Materials.push_back(blue);
    Materials.push_back(black);
    Materials.push_back(wood);
    Materials.push_back(glass);
    Materials.push_back(liquid);
    Materials.push_back(tin);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_texture_cube(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;
    vector<vec2> uvCoords;
    vector<ivec3> indices;
    vector<vec3> tangents;
    vector<vec3> bitangents;

    // Define 3D vertices for cube
    vertices = {
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),    // front
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),   // back
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),
            vec4(0.5f, -0.5f, -0.5f, 1.0f),   // left
            vec4(0.5f, 0.5f, -0.5f, 1.0f),
            vec4(0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f, -0.5f,  0.5f, 1.0f),
            vec4(0.5f, -0.5f, -0.5f, 1.0f),
            vec4(-0.5f,  -0.5f, -0.5f, 1.0f),   // right
            vec4(-0.5f,  -0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f, -0.5f, 1.0f),
            vec4(-0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),   // top
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),   // bottom
            vec4(0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f)
    };

    normals = {
            vec3(0.0f,  0.0f, 1.0f),    // Front
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(1.0f,  0.0f, -1.0f),   // Back
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(1.0f,  0.0f, 0.0f),    // Left
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),    // Right
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),     // Top
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),     // Bottom
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f)
    };

    uvCoords = {
            vec2(1.0f, 0.0f),    // front
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 0.0f),    // back
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 0.0f),    // left
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 0.0f),    // right
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 0.0f),    // top
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 0.0f),    // bottom
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 1.0f),
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f)
    };

    // Define face indices (ensure proper orientation)
    indices = {
            {0, 1, 2},     // Top
            {2, 3, 0},
            {4, 5, 6},     // Front
            {6, 7, 4},
            {8, 9, 10},    // Back
            {10, 11, 8},
            {12, 13, 14},  // Left
            {14, 15, 12},
            {16, 17, 18},  // Right
            {18, 19, 16},
            {20, 21, 22},  // Bottom
            {22, 23, 20},
    };

    // Set number of vertices
    numVertices[obj] = vertices.size();

    computeTangentBasis(vertices, uvCoords, normals, tangents, bitangents);

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tangCoords*numVertices[obj], tangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*bitangCoords*numVertices[obj], bitangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 2.8f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };
    LightProperties redSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //specular
            vec4(2.0f, 1.8f, 2.0f, 1.0f),  //position
            vec4(-1.0f, 0.0f, -1.0f, 0.0f), //direction
            20.0f,   //cutoff
            20.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    Lights.push_back(whitePointLight);
    Lights.push_back(redSpotLight);

    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_mirror(GLuint m_texid ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[m_texid]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[m_texid]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void build_frame(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;

    // Create wireframe for mirror
    vertices = {
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f)
    };

    normals = {
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
    };

    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_shadows( ) {
    // Generate new framebuffer and corresponding texture for storing shadow distances
    glGenFramebuffers(1, &ShadowBuffer);
    glGenTextures(1, &TextureIDs[ShadowTex]);
    // Bind shadow texture and only store depth value
    glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureIDs[ShadowTex], 0);
    // Buffer is not actually drawn into since only for creating shadow texture
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void build_textures( ) {

    // Create textures and activate unit 0
    glGenTextures( NumTextures,  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    load_texture(blankFile, Blank, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(woodFile, Wood, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(carpetFile, Carpet, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(roofFile, Roof, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(doorFile, Door, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(windowFile, Widow, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(carpetNormFile, CarpetNorm, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(roofNormFile, RoofNorm, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(doorNormFile, DoorNorm, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
    load_texture(woodNormFile, WoodNorm, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        camera_angle -= 0.1f;
    } else if (key == GLFW_KEY_D) {
        camera_angle += 0.1f;
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        dir = center - eye;
        eye = eye + (dir * stepSize);
    }
    else if (key == GLFW_KEY_S)
    {
        dir = center - eye;
        eye = eye - (dir * stepSize);
    }

    //light controls
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        if (lightOn[WhitePointLight] == 1) {
            lightOn[WhitePointLight] = 0;
            swtich2_ang = -45.0f;
        }else {
            lightOn[WhitePointLight] = 1;
            swtich2_ang = 45.0f;
        }
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        if (lightOn[WhiteSpotLight] == 1) {
            lightOn[WhiteSpotLight] = 0;
            swtich3_ang = -45.0f;
        }else {
            lightOn[WhiteSpotLight] = 1;
            swtich3_ang = 45.0f;
        }
    }

    //fan control
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (spin) {
            spin = false;
            swtich1_ang = -45.0f;
        }else {
            spin = true;
            swtich1_ang = 45.0f;
        }
    }

    //blinds control
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        if (blinds) {
            blinds = false;
        }else {
            blinds = true;
        }
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

// Debug shadow renderer
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(debug_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

#include "utilfuncs.cpp"
