#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/Structs.hpp"
#include "frontend/raylib/state/UI.hpp"
#include "mpris/Service.hpp"

namespace frontend::raylib::input
{

class Handler
{
public:
  void handle(audio::Service& audio, state::UI& ui, const RaylibConfig& cfg, mpris::Service* mpris);
};

} // namespace frontend::raylib::input
