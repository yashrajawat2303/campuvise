#include "images.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
namespace images {
namespace fs = std::filesystem;
static std::string extensionOf(const std::string& name) {
 auto dot=name.find_last_of('.'); if(dot==std::string::npos) return ".bin";
 std::string ext=name.substr(dot); if(ext.size()>8) return ".bin";
 std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return (char)tolower(c);});
 if(ext!=".jpg"&&ext!=".jpeg"&&ext!=".png"&&ext!=".webp"&&ext!=".gif") return ".bin";
 return ext;
}
std::string saveImage(int id,const std::string& bytes,const std::string& name){
 fs::create_directories("uploads"); auto path="uploads/report_"+std::to_string(id)+extensionOf(name);
 std::ofstream out(path,std::ios::binary); if(!out) return {}; out.write(bytes.data(),bytes.size()); return out?path:"";
}
std::string getImagePath(int id){ if(!fs::exists("uploads"))return {}; for(auto& p:fs::directory_iterator("uploads")){auto n=p.path().filename().string(); if(n.rfind("report_"+std::to_string(id)+".",0)==0)return p.path().string();} return {}; }
bool imageExists(int id){return !getImagePath(id).empty();}
bool deleteImage(int id){auto p=getImagePath(id);return p.empty()||fs::remove(p);}
}
