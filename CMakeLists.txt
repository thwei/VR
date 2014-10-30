cmake_minimum_required(VERSION 2.8)
 
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

PROJECT(volume_rendering)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "-w -fopenmp")

#########################################################
# Include OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################
# Include GLEW
#########################################################
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
#link_directories(${GLEW_LIBRARY_DIRS})
#add_definitions(${GLEW_DEFINITIONS})
link_libraries(${GLEW_LIBRARY})

if(NOT GLEW_FOUND)
    message(ERROR " GLEW not found!")
endif(NOT GLEW_FOUND)

#########################################################
# Include VTK
#########################################################
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#########################################################
# Include QT4
#########################################################
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

#find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
#find_package(Qt4Widgets)
#qt4_use_modules(QtOpenGL)


########################################################
# CUDA
########################################################
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3 -gencode arch=compute_22,code=sm_22)
find_package(CUDA QUIET REQUIRED)
CUDA_ADD_LIBRARY(RenderLib volumeRender_kernel.cu)
include_directories( ${CUDA_TOOLKIT_ROOT_DIR}/include  ${CUDA_SDK_ROOT_DIR}/common/inc )
link_directories(${CUDA_TOOLKIT_ROOT_DIR}/lib/x64 ${CUDA_SDK_ROOT_DIR}/common/lib/x64 )
#LINUX:
link_directories(${CUDA_TOOLKIT_ROOT_DIR}/lib64)

########################################################

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CXX_FILES *.cpp)

QT4_WRAP_UI(UISrcs ${UI_FILES})
QT4_WRAP_CPP(MOCSrcs ${QT_WRAP})
add_executable(volume_rendering ${CXX_FILES} ${UISrcs} ${MOCSrcs} ${LIBPCA_LIBRARIES} ${QT_WRAP} )

#########################################################
# Link libraries
#########################################################
target_link_libraries(volume_rendering ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY} RenderLib)

