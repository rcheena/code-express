# This build script should work fine with 
#   MinGW 4.5 
#   Visual Studio 2008 Express


cmake_minimum_required(VERSION 2.6)

project(CodeExpress)

add_definitions(-DUNICODE -D_UNICODE)

# Setup resource compiler of MinGW.
# MinGW g++/gcc don't call winres.exe automatically.
if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>") 
endif()        


set(ce_sources
    snippet_table.cpp
    code_express.cpp
    resource.rc
    )


add_library(CodeExpress MODULE ${ce_sources})

# Statically link against MinGW C/C++ runtime.
if(MINGW)
    set_target_properties(CodeExpress PROPERTIES
        PREFIX              ""
        LINK_FLAGS          "-static-libgcc -static-libstdc++"
        LINK_FLAGS_RELEASE  "-s"
        )
endif()


install(TARGETS CodeExpress
    RUNTIME DESTINATION "${CMAKE_SOURCE_DIR}/bin/plugins"
    LIBRARY DESTINATION "${CMAKE_SOURCE_DIR}/bin/plugins"
    )
    
install(DIRECTORY config/
    DESTINATION "${CMAKE_SOURCE_DIR}/bin/plugins/Config/CodeExpress"
    )
    
#
# install(TARGETS CodeExpress
#     RUNTIME DESTINATION "$ENV{PROGRAMFILES}/Notepad++/plugins"
#     LIBRARY DESTINATION "$ENV{PROGRAMFILES}/Notepad++/plugins"
#     )
#
