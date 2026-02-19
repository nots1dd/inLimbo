#pragma once

#include "Store.hpp"
#include <mutex>

namespace telemetry
{

class Sink
{
public:
  void push(const Event& ev);

  auto snapshot() const -> Store;

private:
  mutable std::mutex mtx;
  Store              store;
};

} // namespace telemetry
