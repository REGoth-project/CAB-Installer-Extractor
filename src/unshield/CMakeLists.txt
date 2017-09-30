project(unshield C)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(GNUInstallDirs)

check_include_files(byteswap.h HAVE_BYTESWAP_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/byteswap.h HAVE_SYS_BYTESWAP_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(fnmatch HAVE_FNMATCH)
check_function_exists(iconv HAVE_ICONV)

add_definitions(-DHAVE_CONFIG_H)
set(USE_OUR_OWN_MD5 1)
configure_file(unshield_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/unshield_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
# Enable CMP0042
if (APPLE)
	set(CMAKE_MACOSX_RPATH ON)
endif (APPLE)
ENDIF("${isSystemDir}" STREQUAL "-1")

add_subdirectory(convert_utf)

if(USE_OUR_OWN_MD5)
    add_subdirectory(md5)
endif()

set(LIBUNSHIELD_HEADES
    "internal.h"
    "libunshield.h"
    "log.h"
    "cabfile.h"
)

set(LIBUNSHIELD_SOURCES
    "bswap.c"
    "component.c"
    "directory.c"
    "file.c"
    "file_group.c"
    "helper.c"
    "libunshield.c"
    "log.c"
)


set(LIBUNSHIELD_LIBRARY_TYPE STATIC)
add_library(libunshield ${LIBUNSHIELD_LIBRARY_TYPE} ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})


find_package(ZLIB)

if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield ${ZLIB_LIBRARIES}  md5 convert_utf)
else()
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(libunshield ${ZLIB_LIBRARIES} convert_utf)
endif()

set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES VERSION 0.0.0 SOVERSION 0)
