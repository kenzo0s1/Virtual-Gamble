#pragma once

#include <atomic>

#include "frontend/libraries/imgui/imgui.h"
#define DIRECTINPUT_VERSION 0x0800

inline std::atomic<bool> initialized_interface = false;
inline std::atomic<float> current_crash_size;
inline std::atomic<bool> state_waiting;

inline ImFont* tahomabig;
inline ImFont* firacode;
inline int _selectedtab = -1;