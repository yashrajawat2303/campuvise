#ifndef IMAGES_H
#define IMAGES_H
#include <string>
namespace images {
std::string saveImage(int reportID, const std::string& bytes, const std::string& originalName);
std::string saveImage(int reportID, const std::string& variant, const std::string& bytes, const std::string& originalName);
std::string getImagePath(int reportID);
std::string getImagePath(int reportID, const std::string& variant);
bool imageExists(int reportID);
bool deleteImage(int reportID);
}
#endif
