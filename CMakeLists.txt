cmake_minimum_required(VERSION 3.20)
project(Spank3d VERSION 0.1.0 LANGUAGES CXX)

option(SPANK3D_BUILD_TESTS "Build portable core tests" ON)

add_library(spank3d_core
    src/math/vec3.cpp
)
target_include_directories(spank3d_core PUBLIC include)
target_compile_features(spank3d_core PUBLIC cxx_std_20)
set_target_properties(spank3d_core PROPERTIES CXX_EXTENSIONS OFF)

if(ANDROID)
    add_library(spank3d_gles
        src/render/gles_renderer.cpp
    )
    target_include_directories(spank3d_gles PUBLIC include)
    target_link_libraries(spank3d_gles PUBLIC spank3d_core GLESv3 EGL)
    target_compile_features(spank3d_gles PUBLIC cxx_std_20)
endif()

if(SPANK3D_BUILD_TESTS)
    enable_testing()
    add_executable(spank3d_core_tests tests/core_tests.cpp)
    target_link_libraries(spank3d_core_tests PRIVATE spank3d_core)
    add_test(NAME spank3d_core_tests COMMAND spank3d_core_tests)
endif()
