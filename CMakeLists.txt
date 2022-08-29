cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(PointCloudPlugin LANGUAGES CXX C)

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

file(GLOB source_SRC
	"computeShader.cpp"
	"debugLog.cpp"
	"dllmain.cpp"
	"loadManager.cpp"
	"undoManager.cpp"
	"pch.cpp"
)

# add the executable
add_library(PointCloudPlugin SHARED 
							 ${source_SRC}
							 # *************** THIRD_PARTY ***************
							 ${laszip_SRC}
							 ${zlib_SRC}
							 ${cnpy_SRC}
)

set_target_properties(PointCloudPlugin PROPERTIES CXX_STANDARD 17)

source_group("Source Files" FILES ${source_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/laszip" FILES ${laszip_SRC})
source_group("Source Files/ThirdParty/zlib" FILES ${zlib_SRC})
source_group("Source Files/ThirdParty/cnpy" FILES ${cnpy_SRC})

# set FEBasicApplication as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PointCloudPlugin)

include_directories(
	"thirdparty/"
	"thirdparty/cnpy"
	"thirdparty/zlib"
)