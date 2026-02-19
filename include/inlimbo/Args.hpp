#pragma once

#include "InLimbo-Types.hpp"

namespace inlimbo
{

struct Args
{
#define ARG(name, type, kind, cli, desc, action) type name{};

#define OPTIONAL_ARG(name, type, cli, desc, action) std::optional<type> name{};

#include "defs/args/Edit.def"
#include "defs/args/General.def"
#include "defs/args/Modify.def"
#include "defs/args/Query.def"

#undef ARG
#undef OPTIONAL_ARG
};

} // namespace inlimbo
