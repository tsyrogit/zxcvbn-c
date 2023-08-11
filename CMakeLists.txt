cmake_minimum_required(VERSION 3.14)

project(zxcvbn-c VERSION 0 LANGUAGES C CXX)

add_executable(dictgen dict-generate.cpp)
target_compile_features(dictgen PUBLIC cxx_std_11)
target_compile_options(dictgen PRIVATE "-O3")

add_compile_options("-Wall" "-Wextra")

find_library(LIBM m)

# list of dictionaries
set(WORDS words-eng_wiki.txt words-female.txt words-male.txt words-passwd.txt words-surname.txt words-tv_film.txt)
list(TRANSFORM WORDS PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

# generate dict-src.h and dict-crc.h
add_custom_command(OUTPUT dict-src.h COMMAND dictgen -o ${CMAKE_CURRENT_BINARY_DIR}/dict-src.h ${WORDS} DEPENDS dictgen ${WORDS} COMMENT "Generating dict-src.h")

add_custom_command(OUTPUT dict-crc.h zxcvbn.dict COMMAND dictgen -b -o ${CMAKE_CURRENT_BINARY_DIR}/zxcvbn.dict -h ${CMAKE_CURRENT_BINARY_DIR}/dict-crc.h ${WORDS} DEPENDS dictgen ${WORDS} COMMENT "Generating dict-crc.h")

# build shared version of library
add_library(zxcvbn-shared SHARED zxcvbn.c dict-crc.h dict-src.h)
target_include_directories(zxcvbn-shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(zxcvbn-shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zxcvbn-shared PUBLIC ${LIBM})
set_target_properties(zxcvbn-shared PROPERTIES 
	POSITION_INDEPENDENT_CODE ON
	OUTPUT_NAME zxcvbn
	VERSION ${CMAKE_PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# build static version of library
add_library(zxcvbn-static STATIC zxcvbn.c dict-crc.h dict-src.h)
target_include_directories(zxcvbn-static PRIVATE ${CMAKE_CURRENT_BINARY_DIR} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zxcvbn-static PUBLIC ${LIBM})
set_target_properties(zxcvbn-static PROPERTIES OUTPUT_NAME zxcvbn)

# in root projects we also want to build/run tests...
if(PROJECT_IS_TOP_LEVEL)

	# C tests
	add_executable(test-internals test-internals.c dict-src.h dict-crc.h zxcvbn.h)
	target_include_directories(test-internals PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(test-internals PUBLIC ${LIBM})
	
	add_executable(test-file test.c zxcvbn.c dict-src.h dict-crc.h zxcvbn.h)
	target_compile_definitions(test-file PRIVATE USE_DICT_FILE)
	target_include_directories(test-file PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(test-file PUBLIC ${LIBM})
	
	add_executable(test-inline test.c zxcvbn.c dict-src.h dict-crc.h zxcvbn.h)
	target_include_directories(test-inline PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(test-inline PUBLIC ${LIBM})
	
	# C++ tests
	file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/zxcvbn.c ${CMAKE_CURRENT_BINARY_DIR}/zxcvbn.cpp SYMBOLIC)
	file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/test.c ${CMAKE_CURRENT_BINARY_DIR}/test.cpp SYMBOLIC)
	
	add_executable(test-c++file ${CMAKE_CURRENT_BINARY_DIR}/test.cpp ${CMAKE_CURRENT_BINARY_DIR}/zxcvbn.cpp dict-src.h dict-crc.h zxcvbn.h)
	target_compile_definitions(test-c++file PRIVATE USE_DICT_FILE)
	target_include_directories(test-c++file PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(test-c++file PUBLIC ${LIBM})
	
	add_executable(test-c++inline ${CMAKE_CURRENT_BINARY_DIR}/test.cpp ${CMAKE_CURRENT_BINARY_DIR}/zxcvbn.cpp dict-src.h dict-crc.h zxcvbn.h)
	target_include_directories(test-c++inline PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(test-c++inline PUBLIC ${LIBM})
	
	# libraries tests
	add_executable(test-shlib test.c)
	target_link_libraries(test-shlib zxcvbn-shared)
	
	add_executable(test-statlib test.c)
	target_link_libraries(test-statlib zxcvbn-static)
	
	# test target  
	add_custom_target(test COMMAND test-internals COMMAND test-file COMMAND test-inline COMMAND test-c++inline COMMAND test-c++file  COMMAND test-shlib COMMAND test-statlib DEPENDS testcases.txt)
endif()
