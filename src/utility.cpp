#include <fstream>
#include <sstream>

#include "utility.hpp"

namespace utility {

std::expected<std::string, std::string>
load_file_content(const std::filesystem::path& filepath) {
    std::ifstream file(filepath);

    if (!file) {
        return std::unexpected("Error: Couldn't open the file: " +
                               filepath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (!file) {
        return std::unexpected("Error: Couldn't read the file: " +
                               filepath.string());
    }

    return buffer.str();
}

}
