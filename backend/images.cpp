#include "images.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace images {
namespace fs = std::filesystem;

static std::string extensionOf(const std::string& name) {
    const auto dot = name.find_last_of('.');
    if (dot == std::string::npos) return {};
    std::string ext = name.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext;
}

bool isSupportedImage(const std::string& name, const std::string& bytes) {
    const auto ext = extensionOf(name);
    const auto* data = reinterpret_cast<const unsigned char*>(bytes.data());
    if ((ext == ".jpg" || ext == ".jpeg") && bytes.size() >= 3)
        return data[0] == 0xff && data[1] == 0xd8 && data[2] == 0xff;
    if (ext == ".png" && bytes.size() >= 8) {
        static const unsigned char signature[] = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
        return std::equal(std::begin(signature), std::end(signature), data);
    }
    if (ext == ".gif" && bytes.size() >= 6)
        return bytes.compare(0, 6, "GIF87a") == 0 || bytes.compare(0, 6, "GIF89a") == 0;
    return ext == ".webp" && bytes.size() >= 12 && bytes.compare(0, 4, "RIFF") == 0 && bytes.compare(8, 4, "WEBP") == 0;
}

static std::string saveAt(const std::string& path, const std::string& bytes) {
    fs::create_directories("uploads");
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return {};
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    out.flush();
    return out ? path : std::string();
}

std::string saveImage(int id, const std::string& bytes, const std::string& name) {
    if (!isSupportedImage(name, bytes)) return {};
    return saveAt("uploads/report_" + std::to_string(id) + extensionOf(name), bytes);
}

std::string saveImage(int id, const std::string& variant, const std::string& bytes, const std::string& name) {
    if (!isSupportedImage(name, bytes) || (variant != "before" && variant != "after")) return {};
    return saveAt("uploads/report_" + std::to_string(id) + "_" + variant + extensionOf(name), bytes);
}

std::string getImagePath(int id) {
    if (!fs::exists("uploads")) return {};
    const auto prefix = "report_" + std::to_string(id) + ".";
    for (const auto& path : fs::directory_iterator("uploads")) {
        if (path.path().filename().string().rfind(prefix, 0) == 0) return path.path().string();
    }
    return {};
}

std::string getImagePath(int id, const std::string& variant) {
    if (variant.empty()) return getImagePath(id);
    if (variant != "before" && variant != "after") return {};
    if (!fs::exists("uploads")) return {};
    const auto prefix = "report_" + std::to_string(id) + "_" + variant + ".";
    for (const auto& path : fs::directory_iterator("uploads")) {
        if (path.path().filename().string().rfind(prefix, 0) == 0) return path.path().string();
    }
    return {};
}

bool deleteImageVariant(int id, const std::string& variant, const std::string& keepPath) {
    if (variant != "before" && variant != "after") return false;
    if (!fs::exists("uploads")) return true;
    const auto prefix = "report_" + std::to_string(id) + "_" + variant + ".";
    const auto keep = fs::path(keepPath).lexically_normal().string();
    bool ok = true;
    for (const auto& path : fs::directory_iterator("uploads")) {
        if (path.path().filename().string().rfind(prefix, 0) != 0) continue;
        if (!keepPath.empty() && path.path().lexically_normal().string() == keep) continue;
        std::error_code error;
        fs::remove(path.path(), error);
        ok = ok && !error;
    }
    return ok;
}

bool imageExists(int id) { return !getImagePath(id).empty(); }

bool deleteImage(int id) {
    if (!fs::exists("uploads")) return true;
    const auto mainPrefix = "report_" + std::to_string(id) + ".";
    const auto variantPrefix = "report_" + std::to_string(id) + "_";
    bool ok = true;
    for (const auto& path : fs::directory_iterator("uploads")) {
        const auto name = path.path().filename().string();
        if (name.rfind(mainPrefix, 0) != 0 && name.rfind(variantPrefix, 0) != 0) continue;
        std::error_code error;
        fs::remove(path.path(), error);
        ok = ok && !error;
    }
    return ok;
}
}
