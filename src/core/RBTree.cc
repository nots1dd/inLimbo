#include "core/RBTree.hpp"
#include "toml/Parser.hpp"

namespace dirsort
{

std::string DIRECTORY_FIELD = std::string(
    parser::parseTOMLField(PARENT_LIB, PARENT_LIB_FIELD_DIR)
);

std::string DEBUG_LOG_PARSE = std::string(
    parser::parseTOMLField(PARENT_DBG, PARENT_DBG_FIELD_TAGLIB_PARSER_LOG)
);

} // namespace dirsort
