# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("Android_3D_3rdPerson_Game")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_definitions(-D_DEBUG)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/libs CACHE PATH "Missing description." FORCE)
FetchContent_Declare(assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG master)
set(ASSIMP_ANDROID_JNIIOSYSTEM ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

add_library(game SHARED
        AndroidOut.cpp
        Animation.cpp
        Animator.cpp
        Bone.cpp
        Camera.cpp
        main.cpp
        Model.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        Utility.cpp)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

include_directories(
        includes
        libs/assimp-src/includes
        
        #PhysX
        includes/physx
        includes/physx_source_foundation
        includes/pxshared/include
)
link_directories(
        static_libs/physx/debug
)
# Configure libraries CMake uses to link your target library.
target_link_libraries(game
        # The game activity
        game-activity::game-activity

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log
        assimp

        #PhysX
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysX_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXCharacterKinematic_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXCommon_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXCooking_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXExtensions_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXFoundation_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXPvdSDK_static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/static_libs/physx/debug/libPhysXVehicle_static.a

        
)