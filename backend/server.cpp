#include "httplib.h"
#include "images.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <cctype>
using namespace std;
static vector<string> split(const string&s){vector<string> v; string x; stringstream ss(s); while(getline(ss,x,'|'))v.push_back(x);return v;}
static string join(const vector<string>&v){string s;for(size_t i=0;i<v.size();++i){if(i)s+='|';s+=v[i];}return s;}
static string esc(const string&s){string o;for(unsigned char c:s){switch(c){case '"':o+="\\\"";break;case '\\':o+="\\\\";break;case '\n':o+="\\n";break;case '\r':o+="\\r";break;case '\t':o+="\\t";break;default:if(c<32)o+=' ';else o+=(char)c;}}return o;}
static vector<vector<string>> load(){vector<vector<string>> out;ifstream f("reports.txt");string l;while(getline(f,l)){auto p=split(l);if(p.size()>=6)out.push_back(p);}return out;}
static void store(const vector<vector<string>>&v){ofstream f("reports.txt");for(auto&p:v)f<<join(p)<<"\n";}
static string jsonReports(){auto rs=load();string j="[";for(size_t i=0;i<rs.size();i++){auto&p=rs[i];if(i)j+=',';while(p.size()<8)p.push_back("");j+="{\"id\":"+p[0]+",\"location\":\""+esc(p[1])+"\",\"wasteType\":\""+esc(p[2])+"\",\"severity\":\""+esc(p[3])+"\",\"description\":\""+esc(p[4])+"\",\"status\":\""+esc(p[5])+"\",\"worker\":\""+esc(p[6])+"\",\"image\":\""+esc(p[7])+"\"}";}return j+"]";}
static int nextID(){int id=1001;ifstream f("next_id.txt");if(f)f>>id;ofstream o("next_id.txt");o<<id+1;return id;}
void runServer(){
 httplib::Server server; server.set_payload_max_length(8*1024*1024);
 server.Get("/", [](const httplib::Request&, httplib::Response& r) {
     ifstream f("../frontend/campuvise_step9_notifications.html");
     if (!f) {
         r.status = 500;
         r.set_content("Frontend file not found", "text/plain");
         return;
     }

     stringstream buffer;
     buffer << f.rdbuf();
     r.set_content(buffer.str(), "text/html");
 });
 server.Get("/api/reports",[](const httplib::Request&,httplib::Response&r){r.set_content(jsonReports(),"application/json");});
 server.Post("/api/reports",[](const httplib::Request&q,httplib::Response&r){
 auto field=[&](const char*n){return q.has_param(n)?q.get_param_value(n):string();};
 if(field("location").empty()||field("wasteType").empty()||field("description").empty()){r.status=400;r.set_content("{\"error\":\"location, wasteType and description required\"}","application/json");return;}
 int id=nextID();vector<string> p={to_string(id),field("location"),field("wasteType"),field("severity"),field("description"),"Pending","",""};
 if (q.form.has_file("image")) {
     auto f = q.form.get_file("image");
     auto path = images::saveImage(id, f.content, f.filename);
     p[7] = path;
 }
 auto rs=load();rs.push_back(p);store(rs);r.status=201;r.set_content("{\"id\":"+to_string(id)+",\"status\":\"Pending\"}","application/json");});
 server.Post("/api/reports/verify",[](const httplib::Request&q,httplib::Response&r){int id=q.has_param("id")?stoi(q.get_param_value("id")):0;auto rs=load();bool ok=false;for(auto&p:rs)if(stoi(p[0])==id&&p[5]=="Pending"){p[5]="Verified";ok=true;}if(ok)store(rs);r.status=ok?200:404;r.set_content(ok?"{\"status\":\"Verified\"}":"{\"error\":\"report not found or not Pending\"}","application/json");});
 server.Post("/api/reports/assign",[](const httplib::Request&q,httplib::Response&r){int id=q.has_param("id")?stoi(q.get_param_value("id")):0;string worker=q.has_param("worker")?q.get_param_value("worker"):"";auto rs=load();bool ok=false;for(auto&p:rs)if(stoi(p[0])==id&&p[5]=="Verified"&&!worker.empty()){p[5]="Assigned";p[6]=worker;ok=true;}if(ok)store(rs);r.status=ok?200:400;r.set_content(ok?"{\"status\":\"Assigned\"}":"{\"error\":\"report must be Verified and worker supplied\"}","application/json");});
 server.Post("/api/reports/complete",[](const httplib::Request&q,httplib::Response&r){int id=q.has_param("id")?stoi(q.get_param_value("id")):0;auto rs=load();bool ok=false;for(auto&p:rs)if(stoi(p[0])==id&&p[5]=="Assigned"){p[5]="Completed";ok=true;}if(ok)store(rs);r.status=ok?200:400;r.set_content(ok?"{\"status\":\"Completed\"}":"{\"error\":\"report must be Assigned\"}","application/json");});
 server.Delete(R"(/api/reports/(\d+))",[](const httplib::Request&q,httplib::Response&r){int id=stoi(q.matches[1]);auto rs=load();auto old=rs.size();rs.erase(remove_if(rs.begin(),rs.end(),[&](auto&p){return stoi(p[0])==id;}),rs.end());if(rs.size()==old){r.status=404;r.set_content("{\"error\":\"not found\"}","application/json");return;}store(rs);images::deleteImage(id);r.set_content("{\"deleted\":true}","application/json");});
 server.Get(R"(/api/images/(\d+))",[](const httplib::Request&q,httplib::Response&r){int id=stoi(q.matches[1]);auto path=images::getImagePath(id);if(path.empty()){r.status=404;return;}ifstream f(path,ios::binary);stringstream b;b<<f.rdbuf();auto ext=filesystem::path(path).extension().string();string mime=ext==".png"?"image/png":ext==".webp"?"image/webp":ext==".gif"?"image/gif":"image/jpeg";r.set_content(b.str(),mime);});
 server.set_error_handler([](const auto&,auto&r){if(r.body.empty())r.set_content("{\"error\":\"request failed\"}","application/json");});
 cout<<"CAMPuVISE API listening at http://localhost:8080\n";server.listen("127.0.0.1",8080);
}
