#pragma once

enum LogLevel {
    LOG_NONE    = 0,
    LOG_INFO    = 1,
    LOG_WARNING = 2,
    LOG_DANGER  = 4,
    LOG_ALL     = LOG_DANGER | LOG_WARNING | LOG_INFO
};