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


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
float deltaTime = 0.0f;
auto lastFrameTime =  std::chrono::high_resolution_clock::now();
#include <chrono>

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
float far  = 1000.0f;
auto near_far = glm::vec2(near, far);
//--[ Ground Plane Setup ]----------------------------------------------------------------------


void Renderer::render()
{
    // Check to see if the surface has changed size. This is _necessary_ to do every frame when
    // using immersive mode as you'll get no other notification that your renderable area has
    // changed.
    updateRenderArea();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = currentFrameTime - lastFrameTime;
    deltaTime = duration.count();
    lastFrameTime = currentFrameTime;

    animator_.UpdateAnimation(deltaTime);



    // clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
    shader_.use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians((float)60), (float)width_ / (float)height_, 0.1f, 100.0f);

    
    
    // view = glm::rotate(view, 20.f, glm::vec3(1,1,0));

    shader_.setMat4("projection", projection);
    
    // camera/view transformation
    glm::mat4 view = camera_.GetViewMatrix();
    shader_.setMat4("view", view);

	for (int i = 0; i < animator_.m_FinalBoneMatrices.size(); ++i)
		shader_.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", animator_.m_FinalBoneMatrices[i]);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f)); // translate it down so it's at the center of the scene
    // model = glm::scale(model, glm::vec3(.01f, .01f, .01f));	// it's a bit too big for our scene, so scale it down
    shader_.setMat4("model", model);
    models_.front().Draw(shader_);

//--[ Ground Plane ]----------------------------------------------------------------------
    GroundPlane.use();
    glUniform1fv(nearFarLoc, 2, glm::value_ptr(near_far));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera_.GetViewMatrix()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(planeVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//--[ Ground Plane ]----------------------------------------------------------------------

    // Present the rendered image. This is an implicit glFlush.
    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
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

    shader_ = Shader("shaders/model.vs", "shaders/model.fs");
    Model ourModel("models/sophie/model.dae");
    auto jogAnimation = new Animation("models/sophie/anim/Idle.dae",&ourModel);
    animator_ = Animator(jogAnimation);

    models_.emplace_back(ourModel);

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
    }
}

bool MoveForward = false;
bool MoveBackward = false;

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
                         << "Pointer Down";
                
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
                    break;
                }
                default:
                    aout << "Unknown MotionEvent Action: " << action;     
            }
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