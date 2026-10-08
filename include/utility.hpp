#pragma once

#include <expected>
#include <filesystem>

namespace utility {

std::expected<std::string, std::string>
load_file_content(const std::filesystem::path& filepath);

}
