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

    // ----------------------------------Extra-----------------------------------------
    PxRigidStatic *groundPlane = PxCreateStatic(*PhysXSDK, 
                                                PxTransform(PxVec3(0)),
                                                PxBoxGeometry(500.f, 0.001, 500.f),
                                                *DefaultMaterialPhysX);
    ScenePhysX->addActor(*groundPlane);

    for (int i = 0; i < 100; ++i)
    {
        float r = (float)rand()/ (float)RAND_MAX;
        float s = (float)rand()/ (float)RAND_MAX;
        float t = (float)rand()/ (float)RAND_MAX;
        
        PxRigidDynamic *Ball = PxCreateDynamic(*PhysXSDK,
                                               PxTransform(PxVec3(r*2,r*100.f,r*11)),
                                               PxSphereGeometry(s),
                                               *DefaultMaterialPhysX, 100.0f);

        Ball->setAngularDamping(r*2);
        Ball->setLinearVelocity(PxVec3(0));
        ScenePhysX->addActor(*Ball);

        PxRigidDynamic *Box = PxCreateDynamic(*PhysXSDK,
                                              PxTransform(PxVec3(r*2,s*10.f,t*11)),
                                              PxBoxGeometry(r*2, s*2, t*2),
                                              *DefaultMaterialPhysX, 100.0f);
        ScenePhysX->addActor(*Box);

    }

    

    PhysXShader = Shader("shaders/simple.vs", "shaders/simple.fs");
    PhysXShader.use();
    glGenVertexArrays(1, &PhysXVAO);
}

void StepPhysics(float dt)
{
    ScenePhysX->simulate(1.0f / 30.0f);
    // ScenePhysX->simulate(dt);
    ScenePhysX->fetchResults(true);
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
