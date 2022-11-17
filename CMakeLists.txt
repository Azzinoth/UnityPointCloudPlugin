cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(PointCloudPlugin LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "/MP")

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# *************** THIRD_PARTY ***************

file(GLOB laszip_SRC
	"thirdparty/laszip/include/laszip_api.cpp"
)

file(GLOB zlib_SRC
	"thirdparty/zlib/adler32.c"
	"thirdparty/zlib/crc32.c"
	"thirdparty/zlib/inffast.c"
	"thirdparty/zlib/inflate.c"
	"thirdparty/zlib/inftrees.c"
	"thirdparty/zlib/zutil.c"
)

file(GLOB cnpy_SRC
	"thirdparty/cnpy/cnpy.cpp"
)

file(GLOB BasicApplicationSubSystem_SRC
	"thirdparty/FEBasicApplication/FEBasicApplication.cpp"
	"thirdparty/FEBasicApplication/FEBasicApplication.h"
	"thirdparty/FEBasicApplication/FEThreadPool.cpp"
	"thirdparty/FEBasicApplication/FEThreadPool.h"
	"thirdparty/FEBasicApplication/FELog.cpp"
	"thirdparty/FEBasicApplication/FELog.h"
	"thirdparty/FEBasicApplication/FETime.cpp"
	"thirdparty/FEBasicApplication/FETime.h"
)

file(GLOB imgui_SRC
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_demo.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_draw.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_impl_glfw.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_impl_opengl3.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_tables.cpp"
	"thirdparty/FEBasicApplication/ThirdParty/imgui/imgui_widgets.cpp"
)

# *************** THIRD_PARTY END ***************

file(GLOB source_SRC
	"computeShader.cpp"
	"computeShader.h"
	"DX11GPU.cpp"
	"DX11GPU.h"
	"dllmain.cpp"
	"Octree.cpp"
	"Octree.h"
	"PointCloud.cpp"
	"PointCloud.h"
	"loadManager.cpp"
	"loadManager.h"
	"undoManager.cpp"
	"undoManager.h"
	#"APIFunctionsTools.cpp"
	#"APIFunctionsTools.h"
	"pch.cpp"
	"pch.h"
)

link_directories("thirdparty/FEBasicApplication/ThirdParty/glew2/lib/$(PlatformTarget)")
link_directories("thirdparty/FEBasicApplication/ThirdParty/GLFW/lib/$(PlatformTarget)")

# add the executable
add_library(PointCloudPlugin SHARED 
							 ${source_SRC}
							 # *************** THIRD_PARTY ***************
							 ${laszip_SRC}
							 ${zlib_SRC}
							 ${cnpy_SRC}
							 ${BasicApplicationSubSystem_SRC}
							 ${imgui_SRC}
)

target_link_libraries(PointCloudPlugin glew32.lib)
target_link_libraries(PointCloudPlugin glfw3.lib)
target_link_libraries(PointCloudPlugin glfw3dll.lib)
target_link_libraries(PointCloudPlugin opengl32.lib)

set_target_properties(PointCloudPlugin PROPERTIES CXX_STANDARD 17)

source_group("Source Files" FILES ${source_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/laszip" FILES ${laszip_SRC})
source_group("Source Files/ThirdParty/zlib" FILES ${zlib_SRC})
source_group("Source Files/ThirdParty/cnpy" FILES ${cnpy_SRC})
source_group("Source Files/ThirdParty/BasicApplication" FILES ${BasicApplicationSubSystem_SRC})
source_group("Source Files/ThirdParty/imgui" FILES ${imgui_SRC})

# set FEBasicApplication as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PointCloudPlugin)

include_directories(
	"thirdparty/"
	"thirdparty/cnpy"
	"thirdparty/zlib"
	"thirdparty/FEBasicApplication/ThirdParty/glew2/include"
	"thirdparty/FEBasicApplication/ThirdParty/"
)