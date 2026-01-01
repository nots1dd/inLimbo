#pragma once

#include <map>
#include <string>
#include <vector>

using Album  = std::string;
using Artist = std::string;
using Genre  = std::string;
using Lyrics = std::string;
using Year   = uint;
using Disc   = uint;
using Track  = uint;

using Artists = std::vector<Artist>;
using Albums  = std::vector<Album>;
using Genres  = std::vector<Genre>;

template <typename T, typename S> using BucketedMap = std::map<T, std::vector<S>>;
