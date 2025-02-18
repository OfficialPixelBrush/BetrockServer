#pragma once

enum LogLevel {
    LOG_NONE    =  0,
    LOG_CHAT    =  1,
    LOG_MESSAGE =  2,
    LOG_INFO    =  4,
    LOG_WARNING =  8,
    LOG_ERROR   = 16,
    LOG_DEBUG   = 32,
    LOG_ALL     = LOG_DEBUG | LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_MESSAGE | LOG_CHAT
};