#ifndef ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
#define ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H

#include <memory>
#include <android/asset_manager.h>
#include <android/imagedecoder.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include "assimp/texture.h"


class TextureAsset {
public:
    /*!
     * Loads a texture asset from the assets/ directory
     * @param assetManager Asset manager to use
     * @param assetPath The path to the asset
     * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
     */
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath, const std::string &assetType);

    static std::shared_ptr<TextureAsset>
    loadAsset(const aiTexture *embeddedTexture, const std::string &assetPath, const std::string &assetType);

    static GLuint 
    UploadTextureToGPU(uint8_t* data, int32_t width, int32_t height);

    static GLuint 
    UploadTextureSTB_Image(uint8_t *data, size_t size);
    static GLuint
    UploadTextureSTB_Image(const char *filename);

    ~TextureAsset();

    /*!
     * @return the texture id for use with OpenGL
     */
    constexpr GLuint getTextureID() const { return id; }
    GLuint id;
    std::string type;
    std::string path;

    inline TextureAsset(GLuint textureId, const std::string &assetPath, const std::string &assetType) : 
    id(textureId), path(assetPath), type(assetType) {}

};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H