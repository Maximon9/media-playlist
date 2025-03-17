set(PLUGIN_AUTHOR "Maximon9")
set(PLUGIN_URL "https://github.com/Maximon9/obs-plugintemplate-master")

cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

set(CMAKE_INSTALL_PREFIX "C:/ProgramData/obs-studio/plugins")

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)

option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

file(GLOB_RECURSE MY_PLUGIN_SOURCES
    "src/*.c"
)

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE ${MY_PLUGIN_SOURCES}
)

#region Add Includes
# Find all header files in the include directory and subdirectories.
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Create a variable to hold the directories.
set(HEADER_DIRS "")

# For each header file, extract its directory.
foreach(header_file ${HEADER_FILES})
    get_filename_component(header_dir ${header_file} DIRECTORY)
    list(APPEND HEADER_DIRS ${header_dir})
endforeach()

# Remove duplicate directories.
list(REMOVE_DUPLICATES HEADER_DIRS)

# Add these directories to your target.
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${HEADER_DIRS})
#endregion

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})