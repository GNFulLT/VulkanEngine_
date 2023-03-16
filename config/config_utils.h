#ifndef CONFIGABLE_H 
#define CONFIGABLE_H

#include "config.h"
#include <memory>
#include "../core/string/string_utils.h"


void register_config(const String& confName,Config* conf);

const std::weak_ptr<Config> get_config_read(const String& confName);

#endif //CONFIGABLE