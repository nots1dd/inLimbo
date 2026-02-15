#include "query/sort/Engine.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"

namespace query::sort
{

static auto dispatchTable() -> const auto&
{
    static const auto table =
        buildDispatch(std::make_index_sequence<TOTAL>{});
    return table;
}

void applyRuntimeSortPlan(SongMap& map, const RuntimeSortPlan& plan)
{
  RECORD_FUNC_TO_BACKTRACE("query::sort::applyRuntimeSortPlan");

  size_t index = static_cast<size_t>(plan.artist) * (AlbumCount * DiscCount * TrackCount) +
                 static_cast<size_t>(plan.album) * (DiscCount * TrackCount) +
                 static_cast<size_t>(plan.disc) * TrackCount + static_cast<size_t>(plan.track);

  dispatchTable()[index](map);

  LOG_DEBUG("query::sort::applyRuntimeSortPlan: Applied sort plan!");
}

} // namespace query::sort
