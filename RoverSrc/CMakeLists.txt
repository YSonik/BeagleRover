# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
#        projectName                                                LanguageUsed
project(Beagle_Rover VERSION 1.0 DESCRIPTION "The final project." LANGUAGES C)

# Compiler options (inherited by sub-folders)
set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -pthread)
add_compile_options(-fdiagnostics-color)

# Enable address sanitizer
# (Comment this out to make your code faster)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# What folders to build
add_subdirectory(hal)
add_subdirectory(app)

