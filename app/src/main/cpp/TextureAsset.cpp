#include "TextureAsset.h"
#include "AndroidOut.h"
#include "Utility.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include "global.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

GLuint 
TextureAsset::UploadTextureToGPU(uint8_t* data, int32_t width, int32_t height)
{
    // Get an opengl texture
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Clamp to the edge, you'll get odd results alpha blending if you don't
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the texture into VRAM
    glTexImage2D(
            GL_TEXTURE_2D, // target
            0, // mip level
            GL_RGBA, // internal format, often advisable to use BGR
            width, // width of the texture
            height, // height of the texture
            0, // border (always 0)
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            data // Data to upload
    );

    // generate mip levels. Not really needed for 2D, but good to do
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureId;
}

GLuint TextureAsset::UploadTextureSTB_Image(uint8_t *data, size_t size)
{
    int32_t width, height, channelCount;
    uint8_t* imageBits = stbi_load_from_memory(data, size, &width,
                            &height, &channelCount, 4);
    GLuint textureId = UploadTextureToGPU(imageBits, width, height);
    stbi_image_free(imageBits);
    return(textureId);
}

GLuint TextureAsset::UploadTextureSTB_Image(const char *filename)
{
    int32_t width, height, channelCount;
    uint8_t* imageBits = stbi_load(filename, 
                        &width,
                        &height, 
                        &channelCount, 
                        4);
    GLuint textureId = UploadTextureToGPU(imageBits, width, height);   
    stbi_image_free(imageBits);
    return(textureId);
}

std::shared_ptr<TextureAsset>
TextureAsset::loadAsset(AAssetManager *assetManager, const std::string &assetPath, const std::string &assetType) {

    aout << "assetPath: " << assetPath << std::endl;
    // Get the image from asset manager
    auto pTextureAsset = AAssetManager_open(
            assetManager,
            assetPath.c_str(),
            AASSET_MODE_BUFFER);

    if (pTextureAsset)
    {
        aout << "loading file with AssetManger and decoding with stb_image.h: " << std::endl;
        size_t fileLength = AAsset_getLength(pTextureAsset);
        std::vector<uint8_t> buf;
        buf.resize(fileLength);
        int64_t readSize = AAsset_read(pTextureAsset, buf.data(), buf.size());
        assert(readSize == buf.size());
        auto textureId = UploadTextureSTB_Image(buf.data(), buf.size());
        AAsset_close(pTextureAsset);
        return std::shared_ptr<TextureAsset>(new TextureAsset(textureId, assetPath, assetType));
        
    } else
    {
        aout << "loading file with stb_image.h: " << std::endl;
        auto fullPath = std::string(EXTERN_ASSET_DIR)+'/'+assetPath;
        
        auto textureId = UploadTextureSTB_Image(fullPath.c_str());
        return std::shared_ptr<TextureAsset>(new TextureAsset(textureId, assetPath, assetType));    
    }
}

std::shared_ptr<TextureAsset>
TextureAsset::loadAsset(const aiTexture *embeddedTexture, const std::string &assetPath, const std::string &assetType) {
    aout << "Getting embeddedTexture from const aiTexture *embeddedTexture " << std::endl;
    aout << "assetPath: " << assetPath << std::endl;

    auto textureId = UploadTextureSTB_Image((uint8_t *)embeddedTexture->pcData, embeddedTexture->mWidth);
    return std::shared_ptr<TextureAsset>(new TextureAsset(textureId, assetPath, assetType));
}

TextureAsset::~TextureAsset() {
    // return texture resources
    glDeleteTextures(1, &id);
    id = 0;
}