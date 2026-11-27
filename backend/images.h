#ifndef IMAGES_H
#define IMAGES_H
#include <string>
namespace images {
bool isSupportedImage(const std::string& originalName, const std::string& bytes);
std::string saveImage(int reportID, const std::string& bytes, const std::string& originalName);
std::string saveImage(int reportID, const std::string& variant, const std::string& bytes, const std::string& originalName);
std::string getImagePath(int reportID);
std::string getImagePath(int reportID, const std::string& variant);
bool deleteImageVariant(int reportID, const std::string& variant, const std::string& keepPath = "");
bool imageExists(int reportID);
bool deleteImage(int reportID);
}
#endif
