#include "telemetry/Sink.hpp"

namespace telemetry
{

void Sink::push(const Event& ev)
{
  std::lock_guard lock(mtx);
  store.onEvent(ev);
}

auto Sink::snapshot() const -> Store
{
  std::lock_guard lock(mtx);
  return store;
}

} // namespace telemetry
