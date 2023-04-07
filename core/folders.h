#ifndef FOLDERS_H
#define FOLDERS_H

#include <string_view>
#include "string/string_utils.h"

constexpr inline static std::string_view RESOURCES_FOLDER_FULL_PATH = "resources/";
constexpr inline static std::string_view DATA_FOLDER_FULL_PATH = "data/";
constexpr inline static std::string_view LOCALES_FOLDER = "locales/";
constexpr inline static std::string_view LOCALES_FOLDER_FULL_PATH = join_v<RESOURCES_FOLDER_FULL_PATH, LOCALES_FOLDER>;
constexpr inline static std::string_view CONFIG_FOLDER = "config/";
constexpr inline static std::string_view CONFIG_FOLDER_FULL_PATH = join_v<DATA_FOLDER_FULL_PATH, CONFIG_FOLDER>;
constexpr inline static std::string_view ASSETS_FOLDER = "assets/";
constexpr inline static std::string_view FONT_FOLDER = "fonts/";
constexpr inline static std::string_view FONT_FOLDER_FULL_PATH = join_v<RESOURCES_FOLDER_FULL_PATH,FONT_FOLDER>;


// FILES

constexpr inline static std::string_view TEXT_ICON = "text_icon.png";
constexpr inline static std::string_view FOLDER_ICON = "folder_icon.png";

constexpr inline static std::string_view TEXT_ICON_PATH = join_v<ASSETS_FOLDER, TEXT_ICON>;
constexpr inline static std::string_view FOLDER_ICON_PATH = join_v<ASSETS_FOLDER, FOLDER_ICON>;
// EMBEDDED FILES
constexpr inline static std::string_view EMBEDDED_FILE_PATH = ":/";
constexpr inline static std::string_view SPLASH_SCREEN_PNG = "splashscreen.png";
constexpr inline static std::string_view SPLASH_SCREEN_PNG_FULL_PATH = join_v<EMBEDDED_FILE_PATH, ASSETS_FOLDER, SPLASH_SCREEN_PNG>;
constexpr inline static std::string_view LOADING_FONT = "loadingFont.ttf";
constexpr inline static std::string_view LOADING_FONT_FULL_PATH = join_v<EMBEDDED_FILE_PATH,FONT_FOLDER_FULL_PATH,LOADING_FONT>;


#endif // FOLDERS_H