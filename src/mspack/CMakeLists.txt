project(MSPack C)

add_definitions(-DMSPACK_NO_DEFAULT_SYSTEM)

file(GLOB files src/*.cpp src/mspack/*.c)
add_library(mspack STATIC ${files})
set_target_properties(mspack PROPERTIES LINKER_LANGUAGE C)