#pragma once

#include "InLimbo-Types.hpp"
#include "core/InodeMapper.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "utils/RBTree.hpp"

namespace helpers::fs
{

void dirWalkProcessAll(const Directory&                                 directory,
                       utils::RedBlackTree<ino_t, utils::rbt::NilNode>& rbt,
                       core::InodeFileMapper& mapper, core::TagLibParser& parser,
                       core::SongTree& songTree);

}
