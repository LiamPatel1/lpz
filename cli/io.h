#pragma once
#include <vector>
#include <fstream>
#include <filesystem>
#include <span>
#include <expected>
#include <string>
#include <cstddef>


inline std::expected<std::vector<uint8_t>, std::string> read_file(const std::filesystem::path& path)
{
    std::error_code e;
    if (!std::filesystem::exists(path, e)) {
        return std::unexpected( "Input file not found: " + path.string());
    }

    auto size = std::filesystem::file_size(path, e);
    if (e) {
        return std::unexpected( "Error getting file size: " + path.string() );
    }

    if (size == 0) return {};
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected( "Error opening file: " + path.string() );
    }

    std::vector<uint8_t> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {

        if (file.gcount() != static_cast<std::streamsize>(size)) {
            return std::unexpected( "Failed to read expected bytes from: " + path.string() );
        }
    }

    return buffer;
}

inline std::expected<void, std::string> write_file(const std::filesystem::path& path, std::span<const uint8_t> data, bool overwrite = false)
{

    if (!overwrite) {
        std::error_code e;
        if (std::filesystem::exists(path, e)) {
            if (e) {
                return std::unexpected("Failed to check file exists: " + path.string());
            }
            return std::unexpected("File exists and overwrite is disabled: " + path.string());
        }
    }

    if (path.has_parent_path()) {
        std::error_code e;
        std::filesystem::create_directories(path.parent_path(), e);
        if (e) {
            return std::unexpected("Failed to create directories for: " + path.string());
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return std::unexpected("Error opening file for writing: " + path.string());
    }

    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());

        if (!file) {
            return std::unexpected("Error while writing: " + path.string());
        }
    }

    return {};
}


