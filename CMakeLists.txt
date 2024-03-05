cmake_minimum_required (VERSION 3.15)
project (REFPROPTests)

# Turn on more warnings in VS and turn off secure deprecation warnings
if(MSVC)
    add_definitions(-W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(/MT)

    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions(-DIS_32_BIT)
    endif ()
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/externals/boost-cmake")

set(CMAKE_CXX_STANDARD 17)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/externals/Catch")

include_directories("${CMAKE_SOURCE_DIR}/externals/REFPROP-headers/")
include_directories("${CMAKE_SOURCE_DIR}/include/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/externals/manyso/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/externals/manyso/externals/MemoryModule")

# Get everything in the src/ directory (always), but not recursive
file(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# The main EXE
add_executable(main ${APP_SOURCES})
# The required dependencies
target_link_libraries(main PUBLIC Boost::filesystem  PRIVATE Catch2WithMain)
add_test(normal_tests main)

if (WIN32)
    # For windows, the MemoryModule too
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/externals/manyso")
    target_link_libraries(main MemoryModule)
endif()
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(main ${CMAKE_DL_LIBS})
endif()