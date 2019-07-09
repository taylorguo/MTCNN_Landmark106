cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/include
        src/main/cpp)
file(GLOB LANDMARKS_SRC src/main/cpp/*.h
        src/main/cpp/*.cpp)
set(LANDMARKS_COMPILE_CODE ${LANDMARKS_SRC})

add_library(libncnn STATIC IMPORTED )
set_target_properties(libncnn
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jnilibs/armeabi-v7a/libncnn.a)
##################################



add_library( # Sets the name of the library.
#        native-lib
        landmark106
        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
#        src/main/cpp/native-lib.cpp
        ${LANDMARKS_COMPILE_CODE})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
#        native-lib

        landmark106
        libncnn
        jnigraphics
        z
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})