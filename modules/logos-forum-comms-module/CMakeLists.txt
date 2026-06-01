cmake_minimum_required(VERSION 3.14)
project(ForumCommsPlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME forum_comms_module
    SOURCES
        src/forum_comms_interface.h
        src/forum_comms_plugin.h
        src/forum_comms_plugin.cpp
        src/ForumCodec.h
)
