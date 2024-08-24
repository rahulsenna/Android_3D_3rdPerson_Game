#pragma once

#include <stdint.h>
#include <float.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <string>
#include <sstream>
#include <iostream>
#include "AndroidOut.h"

extern  AAssetManager *g_AssetManager;
extern  android_app *g_App;

#define EXTERN_ASSET_DIR "/sdcard/Android/obb/com.example.game"
#define FULL_PATH(path) "/sdcard/Android/obb/com.example.game/" path

#define ASYNC_ASSET_LOADING 1

#include <chrono>

#define TIME_BLOCK(name) \
    struct Timer_##name { \
        std::chrono::high_resolution_clock::time_point start; \
        std::string timer_name; \
        Timer_##name(const char* name) : start(std::chrono::high_resolution_clock::now()), timer_name(name) {} \
        ~Timer_##name() { \
            auto end = std::chrono::high_resolution_clock::now(); \
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); \
            aout << timer_name << " took " << duration.count() << " microseconds." << std::endl; \
        } \
    } timer_instance_##name(#name);

#define CHECK_GL { GLenum glStatus = glGetError(); if( glStatus != GL_NO_ERROR ) { aout << "File: " << __FILE__ << " " << "Line: " << __LINE__ << " " << "OpenGL error: " << openglGetErrorString( glStatus ) << std::endl; } }
static std::string openglGetErrorString(GLenum status)
{
	std::stringstream ss;

	switch (status)
	{
	case GL_INVALID_ENUM:
		ss << "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		ss << "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		ss << "GL_INVALID_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		ss << "GL_OUT_OF_MEMORY";
		break;
	default:
		ss << "GL_UNKNOWN_ERROR" << " - " << status;
		break;
	}

	return ss.str();
}
