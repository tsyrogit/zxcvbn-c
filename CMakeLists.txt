cmake_minimum_required(VERSION 3.16)

project(zxcvbn)

# Force to compile for x86_64 architecture till full support of arm64 for macOS
if(APPLE AND NOT IOS)
    set(CMAKE_OSX_ARCHITECTURES x86_64)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options($<$<CONFIG:Release>:/O2>)
else()
    add_compile_options($<$<CONFIG:Release>:-O2>)
endif()

if(ANDROID)
    add_compile_options(-fsigned-char)
    add_compile_options($<$<CONFIG:Release>:-Os>)
    add_link_options($<$<CONFIG:Release>:-s>)
endif()

if(EMSCRIPTEN)
    add_compile_options(-fexceptions)
    add_compile_options($<$<CONFIG:Release>:-Os>)
    add_link_options(-fexceptions)
    add_link_options($<$<CONFIG:Release>:-s>)
endif()

set(ZXCVBN_VERSION 2.4)
set(ZXCVBN_NAME zxcvbn-c-${ZXCVBN_VERSION})
set(ZXCVBN_DIR ${CMAKE_CURRENT_LIST_DIR})

add_executable(dictgen
    ${ZXCVBN_DIR}/dict-generate.cpp
)

set(WORDS
    words-eng_wiki.txt
    words-female.txt
    words-male.txt
    words-passwd.txt
    words-surname.txt
    words-tv_film.txt
)

add_custom_command(OUTPUT ${ZXCVBN_DIR}/dict-src.h
    COMMAND dictgen -o dict-src.h ${WORDS}
    WORKING_DIRECTORY ${ZXCVBN_DIR}
    COMMENT "Building the dictionary header..."
)

add_custom_target(dictionary DEPENDS ${ZXCVBN_DIR}/dict-src.h)
add_dependencies(dictionary dictgen)

# On Windows 'stdafx.h' is expected, empty will do
add_custom_command(OUTPUT ${ZXCVBN_DIR}/stdafx.h
    COMMAND ${CMAKE_COMMAND} -E touch ${ZXCVBN_DIR}/stdafx.h
)
add_custom_target(stdafx DEPENDS ${ZXCVBN_DIR}/stdafx.h)

add_library(zxcvbn
    ${ZXCVBN_DIR}/dict-src.h
    ${ZXCVBN_DIR}/zxcvbn.h
    ${ZXCVBN_DIR}/zxcvbn.c
)
add_dependencies(zxcvbn dictionary)
if(MSVC)
add_dependencies(zxcvbn stdafx)
endif()
target_include_directories(zxcvbn PRIVATE ${ZXCVBN_DIR})

if(EMSCRIPTEN)
    set(ZXCVBN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/web)
elseif(MSVC)
    set(ZXCVBN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/windows/$<IF:$<CONFIG:Debug>,Debug,Release>)
    set_target_properties(zxcvbn PROPERTIES
        ARCHIVE_OUTPUT_NAME libzxcvbn)
elseif(IOS)
    set(ZXCVBN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/ios)
elseif(APPLE)
    set(ZXCVBN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/macos)
elseif(ANDROID)
    set(ZXCVBN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/android/${ANDROID_ABI})
else()
    message(FATAL_ERROR "Unknown target platform")
endif()

set_target_properties(zxcvbn PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ZXCVBN_OUTPUT_DIR})

