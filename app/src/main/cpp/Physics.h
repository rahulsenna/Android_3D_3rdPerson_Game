#pragma once

#include "PxPhysicsAPI.h"
using namespace physx;

Shader PhysXShader;
GLuint PhysXVAO;

struct ControlledActorDesc
{

    PxControllerShapeType::Enum   Type;
    PxExtendedVec3                Position;
    float                         SlopeLimit;
    float                         ContactOffset;
    float                         StepOffset;
    float                         InvisibleWallHeight;
    float                         MaxJumpHeight;
    float                         Radius;
    float                         Height;
    float                         CrouchHeight;
    float                         ProxyDensity;
    float                         ProxyScale;
    float                         VolumeGrowth;
    PxUserControllerHitReport    *ReportCallback;
    PxControllerBehaviorCallback *BehaviorCallback;
};

struct physx_actor_entity
{
    PxRigidActor *actorPtr;
    PxU32         actorId;
};

struct physics_engine
{
    PxFoundation           *Foundation;
    PxPhysics              *SDK;
    PxCooking              *Cooking;
    PxDefaultCpuDispatcher *Dispatcher;
    PxScene                *Scene;
    PxPvd                  *Pvd;
};

#define PVD_HOST "192.168.1.100"//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
#define PX_RELEASE(x) \
    if (x)            \
    {                 \
        x->release(); \
        x = NULL;     \
    }
PxFoundation           *FoundationPhysX;
PxPhysics              *PhysXSDK;
PxCooking              *CookingPhysX;
PxDefaultCpuDispatcher *DispatcherPhysX;
PxScene                *ScenePhysX;
PxPvd                  *PvdPhysX;
PxMaterial             *DefaultMaterialPhysX;

PxDefaultAllocator     gAllocator;
PxDefaultErrorCallback gErrorCallback;

std::vector<physx_actor_entity> physx_actors;

void createDynamic(const physx::PxTransform &t, const physx::PxGeometry &geometry, const physx::PxVec3 &velocity = physx::PxVec3(0))
{
    static physx::PxU32 dynamicCounter = 0;

    physx::PxRigidDynamic *dynamic = PxCreateDynamic(*PhysXSDK, t, geometry, *DefaultMaterialPhysX, 100.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    ScenePhysX->addActor(*dynamic);

    physx_actors.push_back({dynamic, dynamicCounter++});
}

void createStack(const physx::PxTransform &t, physx::PxU32 size, physx::PxReal halfExtent)
{
    physx::PxShape *shape = PhysXSDK->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *DefaultMaterialPhysX);

    static physx::PxU32 stackCounter = 0;
    for (physx::PxU32   i            = 0; i < size; i++)
    {
        for (physx::PxU32 j = 0; j < size - i; j++)
        {
            physx::PxTransform    localTm(physx::PxVec3(physx::PxReal(j *
                                                                      2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
            physx::PxRigidDynamic *body = PhysXSDK->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            physx::PxRigidBodyExt::updateMassAndInertia(*body, 20.0f);
            ScenePhysX->addActor(*body);

            physx_actors.push_back({body, stackCounter++});
        }
    }

    shape->release();
}
#include "BasicGeometryMesh.h"
#include "TextureAsset.h"


GLMeshData myBox;
GLMeshData mySphere;
Shader *GeomShader;
Shader *GeomShaderInstanced;
Shader *GeomShaderInstacedWithMultiTexture;
GLuint uMVP_BasicGeom;
GLuint uTextureBasicGeom;
GLuint texture_crate   = -1;
GLuint texture_checker = -1;
const int num_ball_textures = 15;
GLuint    texIds[num_ball_textures]; // 15 ball

GLuint uViewProjInstancedGeom;
GLuint uTextureInstancedGeom;
GLuint uViewProjInstancedMultiTex;

Shader *GeomShaderAtlas;
GLuint uViewProjnstancedAtlas;
GLuint uAtlasTexture;
GLuint TextureAtlasPoolBalls;

void InitPhysics()
{

    FoundationPhysX = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    PvdPhysX                  = PxCreatePvd(*FoundationPhysX);
    PxPvdTransport *transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 1000000);
    auto PvdConnected = PvdPhysX->connect(*transport, PxPvdInstrumentationFlag::eALL);
    aout << "PvdConnected: " << PvdConnected  << std::endl;

    PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *FoundationPhysX, PxTolerancesScale(), true, PvdPhysX);

    // Cooking
    CookingPhysX = PxCreateCooking(PX_PHYSICS_VERSION, *FoundationPhysX, PxCookingParams(PxTolerancesScale()));
    if (!CookingPhysX)
    {
        return;
    }
    aout << "CookingPhysX: " << CookingPhysX  << std::endl;

    //Cooking End

    PxSceneDesc sceneDesc(PhysXSDK->getTolerancesScale());
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

    sceneDesc.gravity       = PxVec3(0.0f, -9.81f, 0.0f);
    DispatcherPhysX         = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = DispatcherPhysX;
    sceneDesc.filterShader  = PxDefaultSimulationFilterShader;
    ScenePhysX              = PhysXSDK->createScene(sceneDesc);
    ScenePhysX->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1);
    ScenePhysX->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);

    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_STATIC, 1);
    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1);
    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

    PxPvdSceneClient *pvdClient = ScenePhysX->getScenePvdClient();

    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    DefaultMaterialPhysX = PhysXSDK->createMaterial(0.5f, 0.5f, 0.6f);
}

void InitPhysicsRenderData()
{
    PhysXShader = Shader("shaders/simple.vs", "shaders/simple.fs");
    PhysXShader.use();
    glGenVertexArrays(1, &PhysXVAO);

    GeomShader = new Shader("shaders/BasicGeom.vs", "shaders/BasicGeom.fs");
    GeomShaderInstanced = new Shader("shaders/BasicGeomInstance.vs", "shaders/BasicGeom.fs");
    GeomShaderInstacedWithMultiTexture = new Shader("shaders/BasicGeomInstance.vs", "shaders/BasicGeomMultiTexture.fs");

    myBox.createBox(4.0f, 4.0f, 4.0f);
    mySphere.createSphere(2.0f, 32, 32);
    uMVP_BasicGeom = glGetUniformLocation(GeomShader->program_, "MVP");
    uTextureBasicGeom = glGetUniformLocation(GeomShader->program_, "myTextureSampler");
    uViewProjInstancedGeom = glGetUniformLocation(GeomShaderInstanced->program_, "VP");
    uViewProjInstancedMultiTex = glGetUniformLocation(GeomShaderInstacedWithMultiTexture->program_, "VP");
    uTextureInstancedGeom = glGetUniformLocation(GeomShaderInstanced->program_, "myTextureSampler");

    GeomShaderAtlas = new Shader("shaders/BasicGeomInstance.vs", "shaders/atlas.fs");
    uViewProjnstancedAtlas = glGetUniformLocation(GeomShaderAtlas->program_, "VP");
    uAtlasTexture = glGetUniformLocation(GeomShaderAtlas->program_, "atlasTexture");
    
    texture_crate = UploadTextureSTB_Image(FULL_PATH("textures/crate.bmp"));
    TextureAtlasPoolBalls =  UploadTextureSTB_Image(FULL_PATH("textures/pool_ball_atlas.png"));

}

void StepPhysics(float dt)
{
    ScenePhysX->simulate(1.0f / 30.0f);
    // ScenePhysX->simulate(dt);
    ScenePhysX->fetchResults(true);
}

float mAccumulator = 0.0f;
float mStepSize = 1.0f / 60.0f;
void *PhysxScratchBuffer = malloc(1024*16*10000); // has to be multiple of 16k

bool advance(float dt)
{
    mAccumulator  += dt;
    if(mAccumulator < mStepSize)
        return false;

    mAccumulator -= mStepSize;

    ScenePhysX->simulate(mStepSize, 0, PhysxScratchBuffer);
    ScenePhysX->fetchResults(true);
    return true;
}



void AddPhysicsStuff()
{
        PxRigidStatic *groundPlane = PxCreateStatic(*PhysXSDK, 
                                                PxTransform(PxVec3(0)),
                                                PxBoxGeometry(500.f, 0.001, 500.f),
                                                *DefaultMaterialPhysX);
    ScenePhysX->addActor(*groundPlane);

       physx::PxReal     stackZ = -20.0f;
    for (physx::PxU32 i      = 0; i < 4; i++)
        createStack(physx::PxTransform(physx::PxVec3(-20, 0, stackZ -= 10.0f)), 10, 2.0f);
}

std::vector<glm::mat4> BoxInstances;
std::vector<glm::mat4> SphereInstances;
std::vector<GLuint> SphereTextures;
bool Shoot = false;

#include <thread>

void PhysXThreaded()
{
    const int MAX_NUM_ACTOR_SHAPES = 128;
    aout << "PhysX Thread: " << std::this_thread::get_id() << std::endl;
    BoxInstances.reserve(1000);
    SphereInstances.reserve(1000);
    SphereTextures.reserve(1000);

    std::vector<glm::mat4> BakBufferBoxInstances;
    std::vector<glm::mat4> BakBufferSphereInstances;
    std::vector<GLuint> BakBufferSphereTextures;

    BakBufferBoxInstances.reserve(1000);
    BakBufferSphereInstances.reserve(1000);
    BakBufferSphereTextures.reserve(1000);
    InitPhysics();
    AddPhysicsStuff();

    float dt = 1.f/60.f;
    auto LastTime = std::chrono::high_resolution_clock::now();
    int frame_cnt = 0;
    while(1)
    {
        auto ThisTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> Duration = ThisTime - LastTime;
        LastTime = ThisTime;
        dt = Duration.count();

        // StepPhysics(dt);
        if (not advance(dt))
        {
            int timeRemaining = 16 - (int)(dt *1000.f);
            std::this_thread::sleep_for(std::chrono::milliseconds(timeRemaining));
            continue;
        }
        
        BakBufferBoxInstances.clear();
        BakBufferSphereInstances.clear();
        BakBufferSphereTextures.clear();

        physx::PxShape *shapes[MAX_NUM_ACTOR_SHAPES];

        for (physx::PxU32 i = 0; i < static_cast<physx::PxU32>(physx_actors.size()); i++)
        {
            const physx::PxU32 nbShapes = physx_actors[i].actorPtr->getNbShapes();
            PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
            physx_actors[i].actorPtr->getShapes(shapes, nbShapes);

            for (physx::PxU32 j = 0; j < nbShapes; j++)
            {
                const physx::PxMat44 shapePose(physx::PxShapeExt::getGlobalPose(*shapes[j], *physx_actors[i].actorPtr));
                glm::mat4 model_matrix = glm::make_mat4(&shapePose.column0.x);

                if (shapes[j]->getGeometry().getType() == physx::PxGeometryType::eBOX)
                {
                    BakBufferBoxInstances.push_back(model_matrix);
                }
                else if (shapes[j]->getGeometry().getType() == physx::PxGeometryType::eSPHERE)
                {
                    BakBufferSphereInstances.push_back(model_matrix);
                    BakBufferSphereTextures.push_back(texIds[physx_actors[i].actorId % 15]);
                }
            }
        }
        BoxInstances.assign(BakBufferBoxInstances.begin(), BakBufferBoxInstances.end());
        SphereInstances.assign(BakBufferSphereInstances.begin(), BakBufferSphereInstances.end());
        SphereTextures.assign(BakBufferSphereTextures.begin(), BakBufferSphereTextures.end());

        

        if (Shoot)
        {
            Shoot = false;
            createDynamic(
                physx::PxTransform(
                physx::PxVec3(camera_.Position.x, camera_.Position.y, camera_.Position.z)), 
                physx::PxSphereGeometry(2),
                physx::PxVec3(camera_.Front.x, camera_.Front.y, camera_.Front.z) * 50.0f);
        }
        

    }
}

void PhysXDebugRender(glm::mat4 &View)
{
    PhysXShader.use();
    PhysXShader.setMat4("projection", projection);
    PhysXShader.setMat4("view", View);
    // PhysXShader.setMat4("model", glm::mat4(1.f));
    // PhysXShader.setMat4("Color", glm::vec4(0, 1, 0, 1));

    const PxRenderBuffer &Buffer = ScenePhysX->getRenderBuffer();
    std::vector<PxVec3>   Vertices;

    for (PxU32 I = 0; I < Buffer.getNbLines(); I++)
    {
        const PxDebugLine &line = Buffer.getLines()[I];
        // render the line
        Vertices.push_back(line.pos0);
        Vertices.push_back(line.pos1);        
    }

    
    glBindVertexArray(PhysXVAO);
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(PxVec3), Vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), (void *) 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, Vertices.size());
}

// render physx shapes
const int MAX_NUM_ACTOR_SHAPES = 128;
void PhysXRender(glm::mat4 &View)
{

    glm::mat4 vp_mat = projection * View;
    
    if (not BoxInstances.empty())
    {
        GeomShaderInstanced->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_crate);
        glUniform1i(uTextureInstancedGeom, 0);

        glUniformMatrix4fv(uViewProjInstancedGeom, 1, GL_FALSE, glm::value_ptr(vp_mat));
        myBox.renderInstanced(BoxInstances);
    }

    // Render spheres
    if (not SphereInstances.empty())
    {
        GeomShaderAtlas->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureAtlasPoolBalls);
        glUniform1i(uAtlasTexture, 0);
            
        glUniformMatrix4fv(uViewProjnstancedAtlas, 1, GL_FALSE, glm::value_ptr(vp_mat));
        mySphere.renderInstanced(SphereInstances);
    };
}