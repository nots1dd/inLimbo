#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/state/Library.hpp"
#include "frontend/raylib/state/UI.hpp"
#include "mpris/Service.hpp"

namespace frontend::raylib::input
{

class Handler
{
public:
  void handle(audio::Service& audio, state::UI& ui, state::Library& lib, mpris::Service* mpris);
};

} // namespace frontend::raylib::input
