#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>


#include "Mesh.h"

#include "Shader.h"
#include "TextureAsset.h"
#include <vector>
#include <map>

#include "BoneInfo.h"

class Animator;  // Forward declaration of class Animator

class Model 
{
public:
    std::map<string, BoneInfo> m_BoneInfoMap; //
    glm::vec3 m_Position = glm::vec3(0);
    glm::vec3 m_Scale = glm::vec3(1);
    glm::vec3 m_Rotate = glm::ivec3(0);
    glm::mat4 m_Transform = glm::mat4(1);
    Animator *m_Animator = 0;
    Shader *m_Shader;
#if ASYNC_ASSET_LOADING
    bool AsyncInit = false;
#endif
    int m_BoneCounter = 0;

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }    

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
    // model data 
    std::vector<std::shared_ptr<TextureAsset>> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>    meshes;
    std::string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path,  glm::vec3 position = glm::vec3(0),
                                    glm::vec3 scale = glm::vec3(1),
                                    glm::vec3 rotate = glm::ivec3(0),
                                    bool gamma = false) : 
                                    gammaCorrection(gamma),
                                    m_Position(position),
                                    m_Scale(scale),
                                    m_Rotate(rotate)
    {
        m_Transform = glm::translate(m_Transform, m_Position);
        m_Transform = glm::scale(m_Transform, m_Scale);
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(uint32_t shader);
    
    
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<std::shared_ptr<TextureAsset>> loadMaterialTextures(const aiScene *scene, aiMaterial *mat, aiTextureType type, std::string typeName);
};
