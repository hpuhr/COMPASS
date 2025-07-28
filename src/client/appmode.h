#pragma once

#include <string>
#include <stdexcept>

enum AppMode
{
    Offline=0,
    LiveRunning,
    LivePaused
};

inline std::string toString(AppMode app_mode)
{
    if (app_mode == AppMode::Offline)
        return "Offline";
    else if (app_mode == AppMode::LiveRunning)
        return "Live:Running";
    else if (app_mode == AppMode::LivePaused)
        return "Live:Paused";
    else
        throw std::runtime_error("Unkown AppMode "+std::to_string((unsigned int)app_mode));
}

enum class AppState
{
    Starting = 0,
    Running,
    Shutdown
};
