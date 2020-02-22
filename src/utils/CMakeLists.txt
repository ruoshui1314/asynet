# Search all source file in current directory
aux_source_directory(. DIR_LIB_SRCS)

# Generate library 
add_library(utils ${DIR_LIB_SRCS})