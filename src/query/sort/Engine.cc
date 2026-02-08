#include "query/sort/Engine.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"

namespace query::sort
{

const DispatchTableType DISPATCH_TABLE = buildDispatchTable();

void applyRuntimeSortPlan(SongMap& map, const RuntimeSortPlan& plan)
{
  RECORD_FUNC_TO_BACKTRACE("query::sort::applyRuntimeSortPlan");

  constexpr auto A = (size_t)metric::ArtistMetric::COUNT;
  constexpr auto B = (size_t)metric::AlbumMetric::COUNT;
  constexpr auto C = (size_t)metric::TrackMetric::COUNT;

  const auto ai = (size_t)plan.artist;
  const auto bi = (size_t)plan.album;
  const auto ci = (size_t)plan.track;

  if (ai >= A || bi >= B || ci >= C)
  {
    LOG_ERROR("Invalid sort plan indices: {} {} {}", ai, bi, ci);
    return;
  }

  auto fn = DISPATCH_TABLE[ai][bi][ci];

  if (!fn)
  {
    LOG_ERROR("Dispatch table entry is null: {} {} {}", ai, bi, ci);
    return;
  }

  LOG_DEBUG("Found dispatch table!");

  fn(map);
}

} // namespace query::sort
