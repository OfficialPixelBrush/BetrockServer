#pragma once

enum LogLevel {
    LOG_NONE    = 0,
    LOG_MESSAGE = 1,
    LOG_INFO    = 2,
    LOG_WARNING = 4,
    LOG_ERROR   = 8,
    LOG_ALL     = LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_MESSAGE
};