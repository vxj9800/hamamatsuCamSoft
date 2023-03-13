# CMakeList.txt : CMake project for camSoft, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project ("camSoft")

# Make sure that dll runtime libraries are used
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add source to this project's executable.
add_executable (camSoft
"dcamsdk4/inc/dcamapi4.h" "dcamsdk4/inc/dcamprop.h"
"dcamMisc/common.cpp" "dcamMisc/common.h" "dcamMisc/console4.h"
"serialib/lib/serialib.cpp" "serialib/lib/serialib.h"

"camProp.cpp" "camProp.h"
"camRec.cpp" "camRec.h"
"liveCap.cpp" "liveCap.h"
"shtrCtrl.cpp" "shtrCtrl.h"
"condExps.cpp" "condExps.h"
"camSoft.cpp" "camSoft.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.15)
  set_property(TARGET camSoft PROPERTY CXX_STANDARD 17)
endif()

# Define locations to the header files of different libraries
target_include_directories(camSoft PUBLIC
"${CMAKE_SOURCE_DIR}/glm"
"${CMAKE_SOURCE_DIR}/glew-2.1.0/include"
"${CMAKE_SOURCE_DIR}/glfw-3.3.8.bin.WIN64/include"
)

# Look for DCAMAPI library
find_library(
	DCAMAPI_LIB REQUIRED
	NAMES dcamapi # Name of the file to look for
	PATHS "${CMAKE_SOURCE_DIR}/dcamsdk4/lib/win64" # Folder to look into
	NO_DEFAULT_PATH # Do not search system default paths
)
message(STATUS "DCAMAPI_LIB: [${DCAMAPI_LIB}]")

# Look for OPENGL library
find_package(OpenGL REQUIRED)

# Look for GLEW library
find_library(
	GLEW_LIB REQUIRED
	NAMES glew32s # Name of the file to look for
	PATHS "${CMAKE_SOURCE_DIR}/glew-2.1.0/lib/Release/x64" # Folder to look into
	NO_DEFAULT_PATH # Do not search system default paths
)
message(STATUS "GLEW_LIB: [${GLEW_LIB}]")

# Look for GLFW library
find_library(
	GLFW_LIB REQUIRED
	NAMES glfw3_mt # Name of the file to look for
	PATHS "${CMAKE_SOURCE_DIR}/glfw-3.3.8.bin.WIN64/lib-vc2022" # Folder to look into
	NO_DEFAULT_PATH # Do not search system default paths
)
message(STATUS "GLFW_LIB: [${GLFW_LIB}]")

# Link all the libraries
target_link_libraries(camSoft PUBLIC
${DCAMAPI_LIB}
${GLEW_LIB}
${GLFW_LIB}
OpenGL::GL
)
