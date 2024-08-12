#pragma once

#include <stdint.h>
#include <float.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

extern  AAssetManager *g_AssetManager;
extern  android_app *g_App;

#define EXTERN_ASSET_DIR "/sdcard/Android/obb/com.example.game"

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
