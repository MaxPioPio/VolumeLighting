cmake_minimum_required(VERSION 3.1.1)
project(volume-lighting)

set(CMAKE_CXX_STANDARD 11)

# source files
set(SOURCES
	src/camera.cpp
	src/controller.cpp
	src/glutils.cpp
	src/main.cpp
	src/mainwindow.cpp
	src/primitives.cpp
	src/renderwidget.cpp
	src/scene.cpp
	src/shadowrenderer.cpp
	src/trackball.cpp
	src/transferfunction.cpp
	src/transfunccanvas.cpp
	src/transfunceditor.cpp
	src/viewwidget.cpp
	src/volumedata.cpp
	src/volumerenderer.cpp
	src/volumerenderprops.cpp
)

# header files
set(HEADERS
	include/camera.hpp
	include/controller.hpp
	include/glutils.hpp
	include/mainwindow.hpp
	include/primitives.hpp
	include/renderwidget.hpp
	include/scene.hpp
	include/shadowrenderer.hpp
	include/trackball.hpp
	include/transferfunction.hpp
	include/transfunccanvas.hpp
	include/transfunceditor.hpp
	include/viewwidget.hpp
	include/volumedata.hpp
	include/volumerenderer.hpp
	include/volumerenderprops.hpp
)

# shader files
set(SHADERS
        glsl/entryExit.vert
		glsl/entryExit.frag
		glsl/globalopacity.frag
		glsl/localopacity.vert
		glsl/localopacity.frag
		glsl/scattering.frag
		glsl/shadow.vert
		glsl/shadow.frag
		glsl/tex3D.vert
		glsl/volume.vert
		glsl/volume.frag
)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

# autolink qt for windows executable
cmake_policy(SET CMP0020 NEW)
# ignore deprecated compile definitions
cmake_policy(SET CMP0043 NEW)

# ressources
set(RESFILE resources.qrc)
qt5_add_resources(RESOURCES ${RESFILE})

# configure executable
add_executable(vollight ${SOURCES} ${HEADERS} ${SHADERS} ${RESOURCES})
qt5_use_modules(vollight Core Widgets Gui)
if (WIN32)
    qt5_use_modules(vollight OpenGL)
endif (WIN32)
target_link_libraries(vollight ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
add_definitions(${PCL_DEFINITIONS} "-DSHADER_PATH=\"${PROJECT_SOURCE_DIR}/glsl/\"")

# copy required dlls on windows
# makro taken from https://gist.github.com/Rod-Persky/e6b93e9ee31f9516261b
macro(qt5_copy_dll APP DLL)
    # find the release *.dll file
    get_target_property(Qt5_${DLL}Location Qt5::${DLL} LOCATION)
    # find the debug *d.dll file
    get_target_property(Qt5_${DLL}LocationDebug Qt5::${DLL} IMPORTED_LOCATION_DEBUG)

    add_custom_command(TARGET ${APP} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${Qt5_${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Qt5_${DLL}Location}> $<TARGET_FILE_DIR:${APP}>)
endmacro()
if (WIN32)
	qt5_copy_dll(vollight Core)
	qt5_copy_dll(vollight Gui)
	qt5_copy_dll(vollight OpenGL)
	qt5_copy_dll(vollight Widgets)
endif (WIN32)
