#ifndef APPMODE_H
#define APPMODE_H

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
        return "AppMode::Offline";
    else if (app_mode == AppMode::LiveRunning)
        return "AppMode::LiveRunning";
    else if (app_mode == AppMode::LivePaused)
        return "AppMode::LivePaused";
    else
        throw std::runtime_error("Unkown AppMode "+std::to_string((unsigned int)app_mode));
}

#endif // APPMODE_H
