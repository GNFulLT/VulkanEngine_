file(GLOB VulkanEngine_CORE_SRC 
	"core/engine.h"
	"core/folders.h"
	"core/scope.h"
	"core/typedefs.h"
	"core/vec2.h"
	"core/version.h"
	"core/io/io_utils.h"
	"core/io/logger.h"
	"core/io/logger.cpp"
	"core/object/object.h"
	"core/object/object_id.h"
	"core/object/variant.h"
	"core/object/property.h"
	"core/object/property.cpp"
	"core/object/type.h"
	"core/object/argument.h"
	"core/serialize/serializable_struct.h"
	"core/serialize/serializable_enum.h"
	"core/string/locales.h"
	"core/string/string_name.h"
	"core/string/string_types.h"
	"core/string/string_utils.h"
	"core/string/string_utils.cpp"
	"core/string/unicodee_char_range.h"
	"core/string/unicode_char_utils.h"
	"core/string/unicode_char_utils.cpp"
	"core/templates/safe_num.h"
	"core/templates/node.h"
	"core/utils_vulkan.h"
)