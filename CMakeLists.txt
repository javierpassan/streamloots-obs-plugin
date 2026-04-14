cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(obs-streamloots VERSION 3.0.0)

set(PLUGIN_AUTHOR "Streamloots")

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

find_package(libobs REQUIRED)

add_library(
  ${CMAKE_PROJECT_NAME} MODULE
  src/plugin-main.cpp
  src/Config.cpp
  src/Config.hpp
  src/server/WSServer.cpp
  src/server/include/WSServer.h
  src/server/WSRequest.cpp
  src/server/include/WSRequest.hpp
  src/use-case/DisplayImage.cpp
  src/use-case/include/DisplayImage.hpp
  src/use-case/DisplayVideo.cpp
  src/use-case/include/DisplayVideo.hpp
  src/use-case/HideCamera.cpp
  src/use-case/include/HideCamera.hpp
  src/use-case/PressKey.cpp
  src/use-case/include/PressKey.hpp
  src/use-case/RotateCamera.cpp
  src/use-case/include/RotateCamera.hpp
  src/use-case/UseCaseManager.cpp
  src/use-case/include/UseCaseManager.hpp
  src/use-case/utils/getSceneItemInScene.hpp
  src/responses/Response.cpp
  src/responses/include/Response.hpp
  src/responses/ResponseError.cpp
  src/responses/include/ResponseError.hpp
  src/requests/RequestTypes.cpp
  src/requests/include/RequestTypes.hpp
  src/requests/RequestBase.cpp
  src/requests/include/RequestBase.hpp
  src/requests/DisplayImageRequest.cpp
  src/requests/include/DisplayImageRequest.hpp
  src/requests/DisplayVideoRequest.cpp
  src/requests/include/DisplayVideoRequest.hpp
  src/requests/HideCameraRequest.cpp
  src/requests/include/HideCameraRequest.hpp
  src/requests/PressKeyRequest.cpp
  src/requests/include/PressKeyRequest.hpp
  src/requests/RotateCameraRequest.cpp
  src/requests/include/RotateCameraRequest.hpp
  src/requests/utils/metadata.hpp
  src/forms/settings-dialog.cpp
  src/forms/settings-dialog.h)

# Import asio

set(ASIO_DIR CACHE FILEPATH "ASIO_DIR library")

add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_include_directories(asio SYSTEM INTERFACE "${ASIO_DIR}/asio/include")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE asio)

# Import Websocketspp

set(CPPWS_DIR CACHE FILEPATH "CPPWS_DIR library")

add_library(Websockets INTERFACE)
target_include_directories(Websockets SYSTEM INTERFACE "${CPPWS_DIR}")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Websockets)

# Import plugin

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# Include directories

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link libraries

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

# Output name

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME "obs-streamloots")
