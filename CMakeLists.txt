cmake_minimum_required(VERSION 3.13) # Needs to be 3.13 to allow nicer source configuration features
project(my_lang)

# Check for present global variables defining different compile environments
# NOTE Mingw does not have a defined global variable so the assumption must be made that if MSVC, CYGWIN and UNIX is not present we are on a mingw toolchain
#	This can still be a mingw 32 bit compile or mingw with win32 api or any other combination that cannot be reliably checked
if(MSVC)
	message(STATUS "Using Microsoft Visual C++")
	add_compile_definitions(MSVC)
	add_compile_options("/diagnostics:caret")
	add_compile_options("/wd4251")
	add_compile_options("/wd26812")
	# since /W3 is placed in compile options by default, it'll throw tons of double flag definitions, this replaces the old flag
	message(STATUS ${CMAKE_CXX_FLAGS})
	string(REGEX REPLACE "/W[1-3]" "/W1" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	message(STATUS ${CMAKE_CXX_FLAGS})
	add_link_options("/IGNORE:4099")
	#add_compile_options("/WL")
elseif(CYGWIN)
	message(FATAL_ERROR "CYGWIN not supported")
elseif(UNIX)
	message(FATAL_ERROR "Unix platform detected currently not supported")
else()
	message(STATUS "Neither MSVC, CYGWIN or unix platform detected assuming MINGW-W64 7.2.0 posix-seh NOTE build will fail if this assumption is not correct")
	add_compile_definitions(_WIN32_WINNT=0x0601)
	add_compile_definitions(WINVER=0x0601)
	set(BUILD_MINGW 1)
endif()

# Required when games include the engine sources and redirects build output to external directory
if("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" STREQUAL "")
	message(STATUS "No runtime directory specified using binary directory")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
	message(STATUS "Runtime output directory specified " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Prevents console window from opening
if(CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
	message(STATUS "Building engine with release flags and running compile optimizations")
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -Wl,-allow-multiple-definition")
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ftree-vectorize -ffast-math -ftree-slp-vectorize ") #-ftree-vectorizer-verbose=1
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -ffast-math -ftree-slp-vectorize") #-fopt-info-vec-missed
	set(BUILD_RELEASE 1)
	# Global compile defines
	add_compile_definitions(DEBUG=0)
	add_compile_definitions(RELEASE=1)

	option(FORCE_SSE42 "Force glm SSE4.2 vectorization?" ON)
	if(FORCE_SSE42)
		add_compile_definitions(GLM_FORCE_SSE42)
	endif()

	add_compile_definitions(BUILD_RELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Building engine with debug flags")
	# Global compile defines
	add_compile_definitions(DEBUG=1)
	add_compile_definitions(RELEASE=0)
	add_compile_definitions(BUILD_DEBUG)
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-allow-multiple-definition")

        # fstack-protector enables stack protection checks that make sure that the stack isn't invalidated
        # Wno-format because %zl throws a warning for unknown format even though it is valid
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -Wno-sign-compare -Wno-unused-variable -Wno-format")
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wno-sign-compare -Wno-unused-variable -Wno-format")
	set(BUILD_DEBUG 1)
else()
	message(FATAL_ERROR "Unsupported build type")
endif()

add_custom_target(CopyExamplePrograms)
add_custom_command(TARGET CopyExamplePrograms POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/example_programs"
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		)

add_executable(mylangc src/main.cpp)
add_dependencies(mylangc CopyExamplePrograms)
add_subdirectory(src)