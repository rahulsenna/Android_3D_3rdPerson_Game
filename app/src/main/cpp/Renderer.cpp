#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>

#include "AndroidOut.h"
#include "Shader.h"
#include "Utility.h"
#include "TextureAsset.h"
#include "Animation.h"
#include "Animator.h"

#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

unordered_map<std::string, Shader*> shaders_;
std::vector<Model> models_;
// Animator *m_Animator;

// camera
bool firstMouse_;
float lastX_=0, lastY_=0;
Camera camera_ = glm::vec3(0,3,5);


#define FONT_HEIGHT 32.0f

//! executes glGetString and outputs the result to logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

/*!
 * @brief if glGetString returns a space separated list of elements, prints each one on a new line
 *
 * This works by creating an istringstream of the input c-style string. Then that is used to create
 * a vector -- each element of the vector is a new element in the input string. Finally a foreach
 * loop consumes this and outputs it to logcat using @a aout
 */
#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

//! Color for cornflower blue. Can be sent directly to glClearColor
#define CORNFLOWER_BLUE 100 / 255.f, 149 / 255.f, 237 / 255.f, 1


Renderer::~Renderer() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

glm::mat4 projection;
glm::mat4 OrthoProjection;

#include "Physics.h"
bool PhysXDebug = false;

#include "Threading.h"

std::mutex ModelMutex;
std::mutex ReadyToUploaMutex;
std::mutex TextureToLoadMutex;
std::mutex GlobalMutex;
std::unordered_map<std::string, int> SeenTexture;
std::unordered_map<std::string, std::shared_ptr<TextureAsset>> TextureStore;
std::queue<TextureAsset*> ReadyToUploadQueue;
std::deque<TextureAsset*> TextureToLoadQueue;
void LoadSingleTextureThreaded(TextureAsset *texture);
extern std::unordered_map<std::string, aiTextureType> TextureTypes;

#include "TextRender.h"
int frame_counter = 0;
std::string fps_str = "";
auto lastFPStime =  std::chrono::high_resolution_clock::now();





float deltaTime = 0.0f;
auto lastFrameTime =  std::chrono::high_resolution_clock::now();

//--[ Ground Plane Setup ]----------------------------------------------------------------------
GLfloat planeVertices[] = {
    // Positions
    1.0f,  1.0f,  // Vertex 0
   -1.0f,  1.0f,  // Vertex 1
   -1.0f, -1.0f,  // Vertex 2
    1.0f, -1.0f,  // Vertex 3
};
GLuint planeIndices[] = {
    0, 1, 2,  // First triangle
    0, 2, 3   // Second triangle
};

GLuint planeVAO, planeVBO, ebo;
Shader GroundPlane;
GLint viewLoc;
GLint projLoc;
GLint nearFarLoc;

float near = 0.1f;
float far  = 10000.0f;
auto near_far = glm::vec2(near, far);
//--[ Ground Plane Setup ]----------------------------------------------------------------------

#include "Content.h"

void InitGroundPlane()
{
//--[ Ground Plane Setup ]----------------------------------------------------------------------
 
    GroundPlane = Shader("shaders/ground_plane.vs", "shaders/ground_plane.fs");
    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);
	
    glGenBuffers(1, &planeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

	 // Generate and bind index buffer
    GLuint planeEbo;
    glGenBuffers(1, &planeEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);
    
	viewLoc = glGetUniformLocation(GroundPlane.program_, "view");
	projLoc = glGetUniformLocation(GroundPlane.program_, "projection");
    nearFarLoc = glGetUniformLocation(GroundPlane.program_, "u_nearfar");
//--[ Ground Plane Setup ]----------------------------------------------------------------------
}

//--[ UI Setup ]----------------------------------------------------------------------



inline bool
RectIntersect(glm::vec4 &dim, float x, float y)
{
    float error = 40.f;
    bool result = (x+error) >= dim.x && (x-error) <= (dim.z+dim.x) && (y+error) >= dim.y && (y-error) <= (dim.w +dim.y);
    return result;
}

GLMeshData UiQuad;
inline glm::mat4 GetQuadMatrix(float x, float y, float width, float height)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    return OrthoProjection*model;
}

inline glm::mat4 GetQuadMatrix(glm::vec4 &dim)
{
    return GetQuadMatrix(dim.x,dim.y, dim.z,dim.q);
}
float WidthF, HeightF;
float Width001, Height001;
float Width001Aspcet;
GLuint ShootBtnImg;
glm::mat4 ShootBtnMat;
glm::vec4 ShootBtnPos;

struct ui
{
    GLuint IMG;
    bool Active = false;
    glm::mat4 Mat;
    glm::vec4 Pos;
    char Label[10] = {0};
    glm::vec3 LabelPos;

    // ui(GLuint img, glm::vec4 pos)
    // {   IMG = img;
    //     Pos = pos;
    //     Mat = GetQuadMatrix(Pos);
    // }
    
};
struct buttons
{
    enum btn
    {
        OrbitCamera= 0x0,
        Shoot,
        JumpBtn,
        D_Up,
        D_Down,
        D_Left,
        D_Right,
        RunBtn,
        Count
    };

    ui   UIs[Count]    = {};
    bool Hold[Count]   = {};
    bool Tapped[Count] = {};
};



buttons Buttons;
Shader *UI_Shader;
GLuint uMVP_UI;
GLuint uTextureUI;
GLuint uOpacityUI;

inline void UiInit()
{

    UiQuad.createQuad();
    UI_Shader = new Shader("shaders/UI_Quad.vs", "shaders/UI_Quad.fs");
    uMVP_UI    = glGetUniformLocation(UI_Shader->program_, "uMVP");
    uTextureUI = glGetUniformLocation(UI_Shader->program_, "uTextureUI");
    uOpacityUI = glGetUniformLocation(UI_Shader->program_, "uOpacityUI");

    Buttons.UIs[buttons::Shoot] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/shoot_btn.png")),
        .Pos = {(89.f*Width001), 50.f*Height001, Width001Aspcet *20.f, Height001*20.f}};

    Buttons.UIs[buttons::OrbitCamera] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/exclaim_btn.png")),
        .Pos = {(2.f*Width001), (50.f*Height001), Width001Aspcet *10.f, Height001*10.f}};

    Buttons.UIs[buttons::JumpBtn] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/red_circle.png")),
        .Pos = {(85.f*Width001), (75.f*Height001), Width001Aspcet *15.f, Height001*15.f},
        .Label = "Jump"};

    Buttons.UIs[buttons::RunBtn] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/run_icon.png")),
        .Pos = {(7.f*Width001), 58.f*Height001, Width001Aspcet *10.f, Height001*10.f}};

    Buttons.UIs[buttons::D_Up] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/up_arrow2.png")),
        .Pos = {(7.f*Width001), 70.f*Height001, Width001Aspcet *10.f, Height001*10.f}};

    Buttons.UIs[buttons::D_Down] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/down_arrow2.png")),
        .Pos = {(7.f*Width001), 84.f*Height001, Width001Aspcet *10.f, Height001*10.f}};

    Buttons.UIs[buttons::D_Left] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/left_arrow2.png")),
        .Pos = {(3.f*Width001), 77.f*Height001, Width001Aspcet *10.f, Height001*10.f}};

    Buttons.UIs[buttons::D_Right] = { 
        .IMG =  UploadTextureSTB_Image(FULL_PATH("textures/ui/right_arrow2.png")),
        .Pos = {(10.9f*Width001), 77.f*Height001, Width001Aspcet *10.f, Height001*10.f}};
    
    for (int BtnIdx = 0; BtnIdx < buttons::Count; ++BtnIdx)
    {
        Buttons.UIs[BtnIdx].Mat = GetQuadMatrix(Buttons.UIs[BtnIdx].Pos);
        if (Buttons.UIs[BtnIdx].Label[0] != 0)
        {
            auto &UI = Buttons.UIs[BtnIdx];

            UI.LabelPos.x = UI.Pos.x + (FONT_HEIGHT*.7);
            UI.LabelPos.y = UI.Pos.y+UI.Pos.w*.6;
            UI.LabelPos.z = UI.Pos.w/(FONT_HEIGHT*5.f);  // Scale   
        }
    }

}
inline void RenderSingleUI(ui &UI)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, UI.IMG);
    
    if (UI.Active)
    {
        glUniform1f(uOpacityUI, 1.f);
    } else
    {
        glUniform1f(uOpacityUI, 0.5f);
    }
    glUniformMatrix4fv(uMVP_UI, 1, GL_FALSE, glm::value_ptr(UI.Mat));
    UiQuad.renderQuad();
}

inline void RenderUI()
{
    if (Buttons.Tapped[buttons::OrbitCamera])
    {
        Buttons.UIs[buttons::OrbitCamera].Active = not Buttons.UIs[buttons::OrbitCamera].Active;
        Buttons.Tapped[buttons::OrbitCamera] = false;
    }
    for (int BtnIdx = buttons::Shoot; BtnIdx < buttons::Count; ++BtnIdx)
    {
        Buttons.UIs[BtnIdx].Active = Buttons.Hold[BtnIdx];
    }


    UI_Shader->use();
    // Disable depth writing for transparent objects
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < buttons::Count; ++i)
    {
        RenderSingleUI(Buttons.UIs[i]);	 
    }
    glDisable(GL_BLEND);

    // Render UI Text (Button Text)
    for (int i = 0; i < buttons::Count; ++i)
    {
        auto &UI = Buttons.UIs[i];
        if (UI.Label[0] != 0)
        { 
            render_text(UI.Label, UI.LabelPos.x, UI.LabelPos.y, UI.LabelPos.z, 1,1,1);
        }
    }
    glDepthMask(GL_TRUE);


}
//--[ UI Setup ]----------------------------------------------------------------------

#if ASYNC_ASSET_LOADING
void LoadSingleTextureThreaded(TextureAsset *texture)
{    
    auto fullPath = std::string(EXTERN_ASSET_DIR) + '/' + texture->path;

    
    texture->buffer = stbi_load(fullPath.c_str(), 
                           &texture->width,
                           &texture->height, 
                           &texture->numColCh, 
                           4);


    if(not texture->buffer)
    {
        aout << "FAILED IMAGE: " + texture->path  << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(ReadyToUploaMutex); // Lock the mutex
    ReadyToUploadQueue.emplace(texture); 
}




void LoadMeshTexturesThreaded(int model_idx, int mesh_idx)
{
    auto &model = models_[model_idx];
    auto &mesh = model.meshes[mesh_idx];

    auto &material = mesh.material;
    auto &textures = mesh.textures;

    for (auto &[TextureTypeStr, TextureType]: TextureTypes)
    {
        for (unsigned int i = 0; i < material->GetTextureCount(TextureType); i++)
        {
            aiString str;
            material->GetTexture(TextureType, i, &str);
            std::string assetPath = model.directory + '/' + str.C_Str();
            
            std::lock_guard<std::mutex> lock(TextureToLoadMutex); // Lock the mutex
            
            SeenTexture[assetPath]++;
            
            if (SeenTexture[assetPath] <= 1)
            {
                auto Text = std::make_shared<TextureAsset> (0, assetPath, TextureTypeStr);
                TextureToLoadQueue.emplace_back(Text.get());
                TextureStore[assetPath] = Text;
            }
            
            assert(TextureStore[assetPath].get() != nullptr);
            textures.emplace_back(TextureStore[assetPath]);
        }
    }
}


void LoadModelThreaded(
        const char *path,
        const char *default_anim_path = "",
        const char *shader = "anim_model",    
        glm::vec3 position = glm::vec3(0),          
        float scale = 1,                          
        glm::vec3 rotate = glm::ivec3(0)            
)
{
    Model model(path, position, glm::vec3(scale), rotate);
    model.m_Shader = shaders_[shader];
    if (default_anim_path)
        model.m_Animator = new Animator(new Animation(default_anim_path, &model));

    int model_idx;
    {
        std::lock_guard<std::mutex> lock(ModelMutex); // Lock the mutex
        models_.emplace_back(model);
        model_idx = models_.size()-1;
    } // Mutex is automatically released here

    for (int mesh_idx =0; mesh_idx < models_[model_idx].meshes.size(); ++mesh_idx) 
        add_task([model_idx, mesh_idx]() { LoadMeshTexturesThreaded(model_idx, mesh_idx); });
    
} 
#endif

void LoadModel(const char *path,
               const char *default_anim_path = "",
               const char *shader = "anim_model",          
               glm::vec3 position = glm::vec3(0),           
               float scale = 1,                                 
               glm::vec3 rotate = glm::ivec3(0)
)
{
#if ASYNC_ASSET_LOADING
      add_task([path, default_anim_path, shader, position,scale,rotate]() {
        LoadModelThreaded(path, default_anim_path, shader, position, scale, rotate);
    });
#else
    Model model(path, position, glm::vec3(scale), rotate);
    model.m_Shader = shaders_[shader];
    if (default_anim_path)
        model.m_Animator = new Animator(new Animation(default_anim_path, &model));
    models_.emplace_back(model);
        
#endif    
}

void AddStuff()
{


    shaders_["anim_model"] = new Shader("shaders/modelAnim.vs", "shaders/model.fs");
    shaders_["model"] = new Shader("shaders/model.vs", "shaders/model.fs");
    
    LoadModel("models/sophie/model.dae",
             "models/sophie/anim/JogForward.dae",
             "anim_model");

    LoadModel("models/van/model.dae",
             "models/van/anim/WalkingHitReaction.dae",
             "anim_model",
             glm::vec3(-2, 0, 0));

    LoadModel("models/arisa/model.dae",
             "models/arisa/anim/StandingRunForward.dae",
             "anim_model",
             glm::vec3(-4,0,0));

    LoadModel("models/maria/model.dae",
             "models/maria/anim/SwordFightOne.dae",
             "anim_model",
             glm::vec3(2,0,0));

    //-------[ Houses ]------------------------------------
    LoadModel("models/HOUSES/morden_wood_cabin/scene.gltf",
             0,
             "model",
             glm::vec3(-4,0,-100), 
             .01);
    

    LoadModel("models/HOUSES/house-home-1/source/model.fbx",
             0,
             "model",
             glm::vec3(-40,0,-100), 
             .01);


}

void Renderer::initRenderer()
{
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);


    // setup any other gl related global states
    glClearColor(CORNFLOWER_BLUE);

    // enable alpha globally for now, you probably don't want to do this in a game
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    stbi_set_flip_vertically_on_load(0);
    const auto processor_count = std::thread::hardware_concurrency();
    aout << "[ processor_count ]: " << processor_count  << std::endl;
    init_thread_pool(processor_count*2);

    InitGroundPlane();
    InitPhysicsRenderData();
    std::thread PhysicsOnSaperateTherad(PhysXThreaded);
    PhysicsOnSaperateTherad.detach();

    AddStuff(); // Content
    updateRenderArea();
    init_text_render_data(width_, height_);
    std::string font_file = std::string(EXTERN_ASSET_DIR)+"/fonts/digital_7_mono.ttf";
    load_font(font_file, FONT_HEIGHT);

    UiInit();
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);

        // make sure that we lazily recreate the projection matrix before we render
        shaderNeedsNewProjectionMatrix_ = true;
        lastX_ = width_ / 2.0f;
        lastY_ = height_ / 2.0f;
        firstMouse_ = true;
        projection = glm::perspective(glm::radians((float)60), (float)width_ / (float)height_, near, far);
        OrthoProjection = glm::ortho(0.0f, (float)width_, (float)height_, 0.0f, -1.0f, 1.0f);
        auto AspectRatio = (float)height_/(float)width_;
        WidthF = (float)width_;
        HeightF = (float)height_;

        Width001Aspcet = (float)width_ * 0.01f * AspectRatio;
        Width001  = (float)width_ * 0.01f;
        Height001 = (float)height_* 0.01f;
    }
}

bool MoveForward = false;
bool MoveBackward = false;
int Holds[10] = {0};

void Renderer::handleInput()
{
    if (MoveForward)
    {
        camera_.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (MoveBackward)
    {
        camera_.ProcessKeyboard(BACKWARD, deltaTime);
    }

    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer)
    {
        // no inputs yet.
        return;
    }
    int32_t rightPointerID = -1;

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++)
    {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto actionPtrIdx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        aout << "Pointer(s): ";


        for (int ptr_idx = 0; ptr_idx < motionEvent.pointerCount; ++ptr_idx)
        {
            auto &pointer = motionEvent.pointers[ptr_idx];
            auto x = GameActivityPointerAxes_getX(&pointer);
            auto y = GameActivityPointerAxes_getY(&pointer);
            // get the x and y position of this event if it is not ACTION_MOVE.        
            // determine the action type and process the event accordingly.

            auto ActionState = action & AMOTION_EVENT_ACTION_MASK;

            if (ActionState == AMOTION_EVENT_ACTION_MOVE)
            {
                
                if (x>width_/3) // right side pad
                {
                    if (firstMouse_)
                    {
                        lastX_ = x;
                        lastY_ = y;
                        firstMouse_ = false;
                    }

                    float xoffset = x - lastX_;
                    float yoffset = lastY_ - y; // reversed since y-coordinates go from bottom to top

                    lastX_ = x;
                    lastY_ = y;

                    camera_.ProcessMouseMovement(xoffset, yoffset);                        
                } else
                {
                    if (y>height_- height_/10)
                    {
                        MoveBackward = true;
                        MoveForward = false;
                    }
                    else
                    {
                        MoveForward = true;
                        MoveBackward = false;   
                    }
                }
                
                aout <<  "(" << pointer.id << ", " << x << ", " << y << ") Pointer Move ";
            }

            if (actionPtrIdx != ptr_idx)
            { 
            	continue;
            }

            auto UpAction = ActionState == AMOTION_EVENT_ACTION_UP || 
                            ActionState == AMOTION_EVENT_ACTION_POINTER_UP ||
                            ActionState == AMOTION_EVENT_ACTION_CANCEL;

            if (UpAction)
            {
                if ( x>width_/3) // right side pad
                    firstMouse_ = true;
                    
                if (x<width_/3) // left side pad
                {
                    MoveForward = false;
                    MoveBackward = false;                    
                }

                aout << "(" << pointer.id << ", " << x << ", " << y << ") Pointer Up ";
                if (Holds[ptr_idx] != -1)
                { 
                    Buttons.Hold[Holds[ptr_idx]] = false;
                    Holds[ptr_idx] = -1;
                }
                continue;
            }

            auto DownAction = ActionState == AMOTION_EVENT_ACTION_DOWN || 
                            ActionState == AMOTION_EVENT_ACTION_POINTER_DOWN;
            {
                for (int i = 0; i < buttons::Count; ++i)
                {
                    Buttons.Hold[i] = false;
                    if (RectIntersect(Buttons.UIs[i].Pos, x,y))
                    {
                        Buttons.Hold[i] = true;
                        Buttons.Tapped[i] = DownAction;
                        Holds[ptr_idx] = i;
                        if (i == buttons::Shoot && DownAction)
                        {
                            Shoot = true;
                        }
                    }	
                }    
            }
            


            

            if (DownAction)
            {
                aout << "(" << pointer.id << ", " << x << ", " << y << ") Pointer Down ";
                
                if (x < width_/3)
                {
                    if (y>height_- height_/10)
                        MoveBackward = true;
                    else
                        MoveForward = true;
                }
            }
#if  0
            switch (action & AMOTION_EVENT_ACTION_MASK)
            {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                {
                    aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                         << "Pointer Down";
                    
                    if (actionPtrIdx == ptr_idx && x < width_/3)
                    {
                        if (y>height_- height_/10)
                            MoveBackward = true;
                        else
                            MoveForward = true;
                        aout << " MoveForward = true; ";
                    }

                    Shoot = true;
                        
                    break;
                }
                case AMOTION_EVENT_ACTION_CANCEL:
                    // treat the CANCEL as an UP event: doing nothing in the app, except
                    // removing the pointer from the cache if pointers are locally saved.
                    // code pass through on purpose.
                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                {
                    aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                         << "Pointer Up";
                    if (actionPtrIdx == ptr_idx &&  x>width_/3) // right side pad
                        firstMouse_ = true;
                    
                    if (actionPtrIdx == ptr_idx && x<width_/3) // left side pad
                    {
                        MoveForward = false;
                        MoveBackward = false;
                        aout << " MoveForward = false; ";
                    }
                    break;
                }

                case AMOTION_EVENT_ACTION_MOVE:
                {
                    if (x>width_/3) // right side pad
                    {
                        if (firstMouse_)
                        {
                            lastX_ = x;
                            lastY_ = y;
                            firstMouse_ = false;
                        }

                        float xoffset = x - lastX_;
                        float yoffset = lastY_ - y; // reversed since y-coordinates go from bottom to top

                        lastX_ = x;
                        lastY_ = y;

                        camera_.ProcessMouseMovement(xoffset, yoffset);                        
                    } else
                    {
                        if (y>height_- height_/10)
                        {
                            MoveBackward = true;
                            MoveForward = false;
                        }
                        else
                        {
                            MoveForward = true;
                            MoveBackward = false;   
                        }
                    }
                    
                    aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                         << "Pointer Move";
#if  0
                
                    // There is no pointer index for ACTION_MOVE, only a snapshot of
                    // all active pointers; app needs to cache previous active pointers
                    // to figure out which ones are actually moved.
                    for (auto index = 0; index < motionEvent.pointerCount; index++)
                    {
                        pointer = motionEvent.pointers[index];
                        x = GameActivityPointerAxes_getX(&pointer);
                        y = GameActivityPointerAxes_getY(&pointer);
                        aout <<  "(" << pointer.id << ", " << x << ", " << y << ")";

                        if (index != (motionEvent.pointerCount - 1)) aout << ",";
                        aout << " ";
                    }
                    aout << "Pointer Move";
#endif  //0
                    break;
                }
                default:
                    aout << "Unknown MotionEvent Action: " << action;     
            }
#endif
            }
        aout << std::endl;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++)
    {
        auto &keyEvent = inputBuffer->keyEvents[i];
        aout << "Key: " << keyEvent.keyCode << " ";
        switch (keyEvent.action)
        {
            case AKEY_EVENT_ACTION_DOWN:
                aout << "Key Down";
                break;
            case AKEY_EVENT_ACTION_UP:
                aout << "Key Up";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // Deprecated since Android API level 29.
                aout << "Multiple Key Actions";
                break;
            default:
                aout << "Unknown KeyEvent Action: " << keyEvent.action;
        }
        aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}

void Renderer::render()
{
    // Check to see if the surface has changed size. This is _necessary_ to do every frame when
    // using immersive mode as you'll get no other notification that your renderable area has
    // changed.
    // updateRenderArea();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = currentFrameTime - lastFrameTime;
    deltaTime = duration.count();
    lastFrameTime = currentFrameTime;

    

    // clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // camera/view transformation
    glm::mat4 view = camera_.GetViewMatrix();

    for (auto &model: models_)
    {
#if ASYNC_ASSET_LOADING
        if (not model.AsyncInit)
        {
            // This has to be done on the Main Thread
            model.AsyncInit = true;
            for (auto &mesh: model.meshes) 
                mesh.setupMesh();
        }
#endif
        model.m_Shader->use();
        if (model.m_Animator)
        {
            model.m_Animator->UpdateAnimation(deltaTime);
            for (int i = 0; i < model.m_Animator->m_FinalBoneMatrices.size(); ++i)
            {
		        model.m_Shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", model.m_Animator->m_FinalBoneMatrices[i]);    
            }
        }
        
        
        model.m_Shader->setMat4("projection", projection);
        model.m_Shader->setMat4("view", view);
        model.m_Shader->setMat4("model", model.m_Transform);
        model.Draw(model.m_Shader->program_);
        
    }

//--[ Ground Plane ]----------------------------------------------------------------------
    GroundPlane.use();
    glUniform1fv(nearFarLoc, 2, glm::value_ptr(near_far));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera_.GetViewMatrix()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(planeVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//--[  Ground Plane ]----------------------------------------------------------------------
    if (PhysXDebug)
        PhysXDebugRender(view);

    PhysXRender(view);

    RenderUI();

#if ASYNC_ASSET_LOADING
    if (not TextureToLoadQueue.empty())
    {
        auto work = TextureToLoadQueue.front();
        add_task([work]() { LoadSingleTextureThreaded(work); });
        TextureToLoadQueue.pop_front();
    }

    if (not ReadyToUploadQueue.empty())
    {
        auto texture = ReadyToUploadQueue.front();
        assert(texture != nullptr);
        texture->id = UploadTextureToGPU(texture->buffer, texture->width, texture->height);
        stbi_image_free(texture->buffer);
        ReadyToUploadQueue.pop();
    }
#endif
    auto thisFPStime =  std::chrono::high_resolution_clock::now();
    frame_counter++;
    std::chrono::duration<float> dt = thisFPStime - lastFPStime;
    if (dt.count() >= 1.0)
    {
        lastFPStime = thisFPStime; 
        fps_str = "FPS: " + std::to_string(frame_counter);
        frame_counter = 0;
    }
    render_text(fps_str, 50.f, 100.f, 1.f, 1.f,0.f,0.f);

    // Present the rendered image. This is an implicit glFlush.
    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
    // std::chrono::duration<float, std::milli> frameTIME = std::chrono::high_resolution_clock::now() - currentFrameTime;
    // aout << "frameTIME: " << frameTIME.count()  << std::endl;
}