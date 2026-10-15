cmake_minimum_required(VERSION 3.16)
project(VirtualCastle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)
find_package(glm QUIET)

find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
endif()

add_executable(VirtualCastle
    src/main.cpp
    src/Shader.cpp
    src/Shader.hpp
    src/Camera.cpp
    src/Camera.hpp
    src/Mesh.cpp
    src/Mesh.hpp
    src/Model.cpp
    src/Model.hpp
    src/GeometryFactory.cpp
    src/GeometryFactory.hpp
    src/Texture.cpp
    src/Texture.hpp
)

target_include_directories(VirtualCastle PRIVATE src)

if(glfw3_FOUND)
    target_link_libraries(VirtualCastle PRIVATE glfw)
else()
    target_include_directories(VirtualCastle PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_link_directories(VirtualCastle PRIVATE ${GLFW3_LIBRARY_DIRS})
    target_link_libraries(VirtualCastle PRIVATE ${GLFW3_LIBRARIES})
endif()

target_link_libraries(VirtualCastle PRIVATE OpenGL::GL GLEW::GLEW)

if(TARGET assimp::assimp)
    target_link_libraries(VirtualCastle PRIVATE assimp::assimp)
else()
    target_link_libraries(VirtualCastle PRIVATE assimp)
endif()

if(TARGET glm::glm)
    target_link_libraries(VirtualCastle PRIVATE glm::glm)
endif()

add_custom_command(TARGET VirtualCastle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:VirtualCastle>/shaders
)

add_custom_command(TARGET VirtualCastle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:VirtualCastle>/assets
)
