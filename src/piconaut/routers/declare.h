#pragma once

#include "piconaut/macro.h"
#include "nvm/macro.h"

//cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(routers)

enum class NodeType { kStatic, kParameter };

NVM_ENUM_CLASS_DISPLAY_TRAIT(NodeType)

PICONAUT_INNER_END_NAMESPACE