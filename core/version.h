#ifndef VERSION_H
#define VERSION_H

#include <string_view>
#include "string/string_utils.h"

#define GNF_VERSION_MAJOR 1

#define GNF_VERSION_MINOR 0

#define GNF_VERSION "" GNF_VERSION_MAJOR "." GNF_VERSION_MINOR

#define GNF_VERSION_AS_HEX 0x1000 * GNF_VERSION_MAJOR + 0x100 * GNF_VERSION_MINOR 

#define GNF_APP GNFEngine

#define GNF_APP_NAME_FULL _STR_XDEF(GNF_APP) "v" GNF_VERSION


#endif // VERSION_H