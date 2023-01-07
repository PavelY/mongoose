#
# project: mongoose
#
cmake_minimum_required(VERSION 3.21)
project(mongoose)
# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

# include_directories(src)
fips_setup()
fips_begin_lib(mongoose)
  fips_files(
    mongoose.h
    mongoose.c
  )
fips_end_lib()
