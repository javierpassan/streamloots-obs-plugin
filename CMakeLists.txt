cmake_minimum_required(VERSION 3.16...3.26)

project(obs-streamloots VERSION 3.0.0)

set(PLUGIN_AUTHOR "Streamloots")

# ── Find OBS libraries directly ────────────────────────────────────────────── We link against a local OBS Studio build
# rather than using the plugin template infrastructure. This is simpler and works with any OBS version.

# libobs
find_path(
  LIBOBS_INCLUDE_DIR
  NAMES obs.h obs-module.h
  HINTS "${LIBOBS_INCLUDE_DIR}" "$ENV{LIBOBS_INCLUDE_DIR}" "${OBS_SOURCE_DIR}/libobs")

find_library(
  LIBOBS_LIB
  NAMES obs obs.lib
  HINTS "${LIBOBS_LIB_DIR}" "$ENV{LIBOBS_LIB_DIR}" "${OBS_BUILD_DIR}/libobs/RelWithDebInfo"
        "${OBS_BUILD_DIR}/libobs/Release" "${OBS_BUILD_DIR}/libobs")

# obs-frontend-api
find_path(
  OBS_FRONTEND_INCLUDE_DIR
  NAMES obs-frontend-api.h
  HINTS "${OBS_SOURCE_DIR}/frontend/api" "${OBS_FRONTEND_INCLUDE_DIR}")

find_library(
  OBS_FRONTEND_LIB
  NAMES obs-frontend-api obs-frontend-api.lib
  HINTS "${OBS_BUILD_DIR}/frontend/api/RelWithDebInfo" "${OBS_BUILD_DIR}/frontend/api/Release"
        "${OBS_BUILD_DIR}/frontend/api" "${OBS_FRONTEND_LIB_DIR}")

if(NOT LIBOBS_INCLUDE_DIR OR NOT LIBOBS_LIB)
  message(FATAL_ERROR "Could not find libobs. Set -DOBS_SOURCE_DIR=C:/obs-studio "
                      "-DOBS_BUILD_DIR=C:/obs-studio/build_x64")
endif()

if(NOT OBS_FRONTEND_INCLUDE_DIR OR NOT OBS_FRONTEND_LIB)
  message(FATAL_ERROR "Could not find obs-frontend-api. Set -DOBS_SOURCE_DIR and -DOBS_BUILD_DIR")
endif()

message(STATUS "Found libobs include: ${LIBOBS_INCLUDE_DIR}")
message(STATUS "Found libobs lib: ${LIBOBS_LIB}")
message(STATUS "Found obs-frontend-api include: ${OBS_FRONTEND_INCLUDE_DIR}")
message(STATUS "Found obs-frontend-api lib: ${OBS_FRONTEND_LIB}")

# ── Find Qt6 ─────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ── Find ASIO (header-only) ──────────────────────────────────────────────────
find_path(
  ASIO_INCLUDE_DIR
  NAMES asio.hpp
  HINTS "${ASIO_INCLUDE_DIR}" "${OBS_DEPS_DIR}/include")
if(NOT ASIO_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find ASIO. Set -DOBS_DEPS_DIR or -DASIO_INCLUDE_DIR")
endif()
message(STATUS "Found ASIO: ${ASIO_INCLUDE_DIR}")

# ── Find WebSocket++ (header-only) ───────────────────────────────────────────
find_path(
  WEBSOCKETPP_INCLUDE_DIR
  NAMES websocketpp/config/asio_no_tls.hpp
  HINTS "${WEBSOCKETPP_INCLUDE_DIR}" "${OBS_DEPS_DIR}/include")
if(NOT WEBSOCKETPP_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find WebSocket++. Set -DOBS_DEPS_DIR or -DWEBSOCKETPP_INCLUDE_DIR")
endif()
message(STATUS "Found WebSocket++: ${WEBSOCKETPP_INCLUDE_DIR}")

# ── Plugin target ────────────────────────────────────────────────────────────
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

# ── Plugin macros header ─────────────────────────────────────────────────────
configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/src
          ${LIBOBS_INCLUDE_DIR}
          ${LIBOBS_INCLUDE_DIR}/.. # for <libobs/obs-module.h> style includes
          ${OBS_BUILD_DIR}/libobs # for generated obsconfig.h
          ${OBS_BUILD_DIR}/config # some OBS versions put it here
          ${OBS_FRONTEND_INCLUDE_DIR}
          ${ASIO_INCLUDE_DIR}
          ${WEBSOCKETPP_INCLUDE_DIR})

# ── Link libraries ───────────────────────────────────────────────────────────
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LIBOBS_LIB} ${OBS_FRONTEND_LIB} Qt6::Core Qt6::Widgets)

# ── Compile definitions ──────────────────────────────────────────────────────
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ASIO_STANDALONE _WEBSOCKETPP_CPP11_STL_)

# ── Qt auto-processing ───────────────────────────────────────────────────────
set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

# ── C++17 ────────────────────────────────────────────────────────────────────
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)

# ── MSVC flags ───────────────────────────────────────────────────────────────
if(MSVC)
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /utf-8)
  # Suppress some noisy websocketpp warnings
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /wd4267 /wd4244 /wd4996)
endif()

# ── Output name (no "lib" prefix on Windows) ─────────────────────────────────
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME "obs-streamloots")

message(STATUS "")
message(STATUS "=== obs-streamloots v${CMAKE_PROJECT_VERSION} ===")
message(STATUS "  OBS source:    ${OBS_SOURCE_DIR}")
message(STATUS "  OBS build:     ${OBS_BUILD_DIR}")
message(STATUS "  Qt6:           ${Qt6_DIR}")
message(STATUS "  ASIO:          ${ASIO_INCLUDE_DIR}")
message(STATUS "  WebSocket++:   ${WEBSOCKETPP_INCLUDE_DIR}")
message(STATUS "")
