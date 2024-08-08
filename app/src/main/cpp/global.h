#pragma once

#include <stdint.h>
#include <float.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

extern  AAssetManager *g_AssetManager;
extern  android_app *g_App;

#define EXTERN_ASSET_DIR "/sdcard/Android/obb/com.example.game"