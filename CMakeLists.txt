cmake_minimum_required(VERSION 3.18)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

set(PROJECT_NAME smallmap)
project(${PROJECT_NAME})

########################################################################
# Sources
set(HEADERS "smallmap.h;tshash.h")
set(SOURCES "main.c;smallmap.c;tshash.c")

source_group("include" FILES ${HEADERS})
source_group("src" FILES ${SOURCES})

set(FILES ${HEADERS} ${SOURCES})

set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

add_executable(${PROJECT_NAME} ${FILES})

if(MSVC)
    set(DEFAULT_C_FLAGS "/DWIN32 /D_WINDOWS /D_UNICODE /DUNICODE /W4 /WX- /nologo /fp:precise /arch:AVX /Zc:wchar_t /TP /Gd /std:c++17 /std:c11")
    if(MSVC_VERSION VERSION_LESS_EQUAL "1900")
        set(DEFAULT_C_FLAGS "${DEFAULT_C_FLAGS} /Zc:__cplusplus /std:c++latest")
    else()
        set(DEFAULT_C_FLAGS "${DEFAULT_C_FLAGS} /Zc:__cplusplus /std:c++17")
    endif()
    set(CMAKE_C_FLAGS "${DEFAULT_C_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1 /Gy /GR- /GS /Gm- /EHsc")
    set(CMAKE_C_FLAGS_RELEASE "/MD /O2 /Oi /GL /GR- /DNDEBUG /EHsc-")

    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            OUTPUT_NAME_DEBUG "${PROJECT_NAME}" OUTPUT_NAME_RELEASE "${PROJECT_NAME}"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

elseif(UNIX)
    set(DEFAULT_C_FLAGS "-Wall -Wextra -O2 -std=c++17 -std=gnu++17 -std=c99 -march=x86-64-v3 -fno-exceptions")
    set(CMAKE_C_FLAGS "${DEFAULT_C_FLAGS}")
elseif(APPLE)
endif()

