# project name
project(Csmania)

cmake_minimum_required(VERSION 2.6.0)

# find and setup Qt4 for this project
find_package(Qt4 REQUIRED)

# g++ compiller flags for debugging
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -Wall")
endif()

# tell cmake to process CMakeLists.txt in these subdirs
add_subdirectory(src)


