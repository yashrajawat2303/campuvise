#include "httplib.h"
#include "images.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
namespace {
using Row = vector<string>;
constexpr size_t FIELD_COUNT = 21;
enum Field { ID, LOCATION, WASTE, SEVERITY, DESCRIPTION, REPORT_STATUS, ASSIGNED, IMAGE,
             FLOOR, ROOM, LANDMARK, REPORTED_BY, BEFORE_IMAGE, AFTER_IMAGE,
             COMPLETION_NOTE, REWORK_NOTE, CREATED_AT, UPDATED_AT, RESOLVED_AT, HISTORY, IS_DEMO };

string jsonEscape(const string& value) {
    string out;
    for (unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: if (c < 32) out += ' '; else out += static_cast<char>(c);
        }
    }
    return out;
}
string encode(const string& value) {
    static const char* hex = "0123456789ABCDEF";
    string out;
    for (unsigned char c : value) {
        if (c >= 32 && c < 127 && c != '%' && c != '|') out += c;
        else { out += '%'; out += hex[c >> 4]; out += hex[c & 15]; }
    }
    return out;
}
int fromHex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
string decode(const string& value) {
    string out;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            int a = fromHex(value[i + 1]), b = fromHex(value[i + 2]);
            if (a >= 0 && b >= 0) { out += static_cast<char>((a << 4) | b); i += 2; continue; }
        }
        out += value[i];
    }
    return out;
}
vector<string> split(const string& s, char delimiter) {
    vector<string> values; string value; stringstream stream(s);
    while (getline(stream, value, delimiter)) values.push_back(value);
    if (!s.empty() && s.back() == delimiter) values.emplace_back();
    return values;
}
Row blankRow() { return Row(FIELD_COUNT); }
vector<Row> load() {
    vector<Row> rows; ifstream file("reports.txt"); string line;
    while (getline(file, line)) {
        auto parts = split(line, '|'); Row row = blankRow();
        bool prefixVersion2 = !parts.empty() && parts.front() == "V2";
        bool suffixVersion2 = !parts.empty() && parts.back() == "V2";
        size_t start = prefixVersion2 ? 1 : 0;
        size_t available = parts.size() - start - (suffixVersion2 && !prefixVersion2 ? 1 : 0);
        size_t count = min(FIELD_COUNT, available);
        bool encoded = prefixVersion2 || suffixVersion2;
        for (size_t i = 0; i < count; ++i) row[i] = encoded ? decode(parts[i + start]) : parts[i + start];
        if (row.size() > REPORT_STATUS && !row[REPORT_STATUS].empty()) {
            if (row[REPORT_STATUS] == "Pending") row[REPORT_STATUS] = "Submitted";
            else if (row[REPORT_STATUS] == "Verified") row[REPORT_STATUS] = "Awaiting Verification";
            else if (row[REPORT_STATUS] == "Completed") row[REPORT_STATUS] = "Resolved";
        }
        if (!row[ID].empty() && !row[LOCATION].empty()) rows.push_back(std::move(row));
    }
    return rows;
}
void store(const vector<Row>& rows) {
    ofstream file("reports.txt", ios::trunc);
    for (const auto& source : rows) {
        Row row = source; while (row.size() < FIELD_COUNT) row.emplace_back();
        // Keep the original console program's ID, status and worker columns readable.
        if (row[REPORT_STATUS] == "Submitted") row[REPORT_STATUS] = "Pending";
        else if (row[REPORT_STATUS] == "Awaiting Verification") row[REPORT_STATUS] = "Verified";
        else if (row[REPORT_STATUS] == "Resolved") row[REPORT_STATUS] = "Completed";
        for (size_t i = 0; i < FIELD_COUNT; ++i) {
            if (i) file << '|';
            file << encode(row[i]);
        }
        file << "|V2\n";
    }
}
long long nowMs() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}
string makeHistory(const string& text, const string& by, long long at) {
    // Percent-encode history components so tabs and newlines stay unambiguous.
    return to_string(at) + "\t" + encode(by) + "\t" + encode(text);
}
void addHistory(Row& row, const string& text, const string& by) {
    if (!row[HISTORY].empty()) row[HISTORY] += '\n';
    row[HISTORY] += makeHistory(text, by, nowMs());
}
string historyJson(const string& value) {
    string out = "["; auto entries = split(value, '\n'); bool first = true;
    for (const auto& entry : entries) {
        auto parts = split(entry, '\t'); if (parts.size() < 3) continue;
        if (!first) out += ','; first = false;
        out += "{\"text\":\"" + jsonEscape(decode(parts[2])) + "\",\"by\":\"" + jsonEscape(decode(parts[1])) + "\",\"at\":" + parts[0] + "}";
    }
    return out + "]";
}
string reportJson(const Row& source) {
    Row row = source; while (row.size() < FIELD_COUNT) row.emplace_back();
    string id = row[ID];
    string image = row[IMAGE].empty() ? "" : (row[IMAGE].rfind("uploads/", 0) == 0 ? "/api/images/" + id : row[IMAGE]);
    string before = row[BEFORE_IMAGE].empty() ? "" : (row[BEFORE_IMAGE].rfind("uploads/", 0) == 0 ? "/api/images/" + id + "/before" : row[BEFORE_IMAGE]);
    string after = row[AFTER_IMAGE].empty() ? "" : (row[AFTER_IMAGE].rfind("uploads/", 0) == 0 ? "/api/images/" + id + "/after" : row[AFTER_IMAGE]);
    auto str = [&](Field f) { return "\"" + jsonEscape(row[f]) + "\""; };
    auto nullableNumber = [&](Field f) { return row[f].empty() ? string("null") : row[f]; };
    return "{\"id\":\"CV-" + jsonEscape(id) + "\",\"location\":" + str(LOCATION) +
      ",\"floor\":" + str(FLOOR) + ",\"room\":" + str(ROOM) + ",\"landmark\":" + str(LANDMARK) +
      ",\"wasteType\":" + str(WASTE) + ",\"severity\":" + str(SEVERITY) +
      ",\"description\":" + str(DESCRIPTION) + ",\"image\":\"" + jsonEscape(image) +
      "\",\"beforeImage\":" + (before.empty() ? "null" : "\"" + jsonEscape(before) + "\"") +
      ",\"afterImage\":" + (after.empty() ? "null" : "\"" + jsonEscape(after) + "\"") +
      ",\"status\":" + str(REPORT_STATUS) + ",\"reportedBy\":" + str(REPORTED_BY) +
      ",\"assignedTo\":" + str(ASSIGNED) + ",\"completionNote\":" + str(COMPLETION_NOTE) +
      ",\"reworkNote\":" + str(REWORK_NOTE) + ",\"createdAt\":" + nullableNumber(CREATED_AT) +
      ",\"updatedAt\":" + nullableNumber(UPDATED_AT) + ",\"resolvedAt\":" + nullableNumber(RESOLVED_AT) +
      ",\"isDemo\":" + string(row[IS_DEMO] == "true" ? "true" : "false") +
      ",\"history\":" + historyJson(row[HISTORY]) + "}";
}
void jsonError(httplib::Response& response, int status, const string& message) {
    response.status = status;
    response.set_content("{\"error\":\"" + jsonEscape(message) + "\"}", "application/json");
}
string param(const httplib::Request& request, const string& name) {
    if (request.has_param(name)) return request.get_param_value(name);
    if (request.form.has_field(name)) return request.form.get_field(name);
    return {};
}
int idFrom(const string& value) {
    string id = value.rfind("CV-", 0) == 0 ? value.substr(3) : value;
    try { return stoi(id); } catch (...) { return 0; }
}
Row* findReport(vector<Row>& rows, int id) {
    for (auto& row : rows) if (idFrom(row[ID]) == id) return &row;
    return nullptr;
}
void sendReport(httplib::Response& response, const Row& row) {
    response.set_content(reportJson(row), "application/json");
}
int nextID() {
    int id = 1001; ifstream input("next_id.txt"); if (input) input >> id;
    ofstream output("next_id.txt"); output << id + 1; return id;
}
string imageUpload(const httplib::Request& request, const string& field, int id, const string& fallback, const string& variant = "") {
    if (!request.form.has_file(field)) return fallback;
    auto uploaded = request.form.get_file(field);
    string path = variant.empty() ? images::saveImage(id, uploaded.content, uploaded.filename)
                                  : images::saveImage(id, variant, uploaded.content, uploaded.filename);
    return path;
}
void touch(Row& row) { row[UPDATED_AT] = to_string(nowMs()); }
}

void runServer() {
    httplib::Server server;
    server.set_payload_max_length(8 * 1024 * 1024);
    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        ifstream file("../frontend/campuvise_step9_notifications.html");
        if (!file) { response.status = 500; response.set_content("Frontend file not found", "text/plain"); return; }
        stringstream content; content << file.rdbuf(); response.set_content(content.str(), "text/html");
    });
    server.Get("/api/reports", [](const httplib::Request&, httplib::Response& response) {
        auto rows = load(); string body = "[";
        for (size_t i = 0; i < rows.size(); ++i) { if (i) body += ','; body += reportJson(rows[i]); }
        response.set_content(body + "]", "application/json");
    });
    server.Post("/api/reports", [](const httplib::Request& request, httplib::Response& response) {
        string location = param(request, "location"), waste = param(request, "wasteType");
        string severity = param(request, "severity"), description = param(request, "description");
        if (location.empty() || waste.empty() || severity.empty() || description.empty()) {
            jsonError(response, 400, "location, wasteType, severity and description are required"); return;
        }
        int id = nextID(); long long now = nowMs(); Row row = blankRow();
        row[ID] = to_string(id); row[LOCATION] = location; row[WASTE] = waste; row[SEVERITY] = severity;
        row[DESCRIPTION] = description; row[REPORT_STATUS] = "Submitted"; row[ASSIGNED] = "";
        row[IMAGE] = imageUpload(request, "image", id, ""); row[FLOOR] = param(request, "floor");
        if (request.form.has_file("image") && row[IMAGE].empty()) { jsonError(response, 500, "Could not save the report photo"); return; }
        row[ROOM] = param(request, "room"); row[LANDMARK] = param(request, "landmark");
        row[REPORTED_BY] = param(request, "reportedBy"); row[CREATED_AT] = row[UPDATED_AT] = to_string(now);
        row[IS_DEMO] = param(request, "isDemo") == "true" ? "true" : "false";
        addHistory(row, "Report submitted", row[REPORTED_BY]);
        auto rows = load(); rows.insert(rows.begin(), row); store(rows);
        response.status = 201; sendReport(response, row);
    });
    server.Post(R"(/api/reports/(CV-)?(\d+)/(start|complete|approve|rework))", [](const httplib::Request& request, httplib::Response& response) {
        int id = idFrom(request.matches[2]); string action = request.matches[3]; auto rows = load(); Row* row = findReport(rows, id);
        if (!row) { jsonError(response, 404, "report not found"); return; }
        string actor = param(request, "actor");
        if (action == "start") {
            if ((*row)[REPORT_STATUS] != "Assigned" || (*row)[ASSIGNED] != actor) { jsonError(response, 400, "This task is not assigned to you"); return; }
            (*row)[REPORT_STATUS] = "In Progress"; addHistory(*row, "Cleaning started", actor);
        } else if (action == "complete") {
            string note = param(request, "note");
            if ((*row)[REPORT_STATUS] != "In Progress" || (*row)[ASSIGNED] != actor) { jsonError(response, 400, "This task is not assigned to you or is not in progress"); return; }
            if (note.empty()) { jsonError(response, 400, "Add a completion note"); return; }
            if (request.form.has_file("beforeImage")) {
                (*row)[BEFORE_IMAGE] = imageUpload(request, "beforeImage", id, "", "before");
                if ((*row)[BEFORE_IMAGE].empty()) { jsonError(response, 500, "Could not save the before photo"); return; }
            }
            (*row)[AFTER_IMAGE] = imageUpload(request, "afterImage", id, "", "after");
            if (request.form.has_file("afterImage") && (*row)[AFTER_IMAGE].empty()) { jsonError(response, 500, "Could not save the after photo"); return; }
            (*row)[COMPLETION_NOTE] = note;
            (*row)[REWORK_NOTE].clear(); (*row)[REPORT_STATUS] = "Awaiting Verification";
            addHistory(*row, "Submitted for verification", actor);
        } else if (action == "approve") {
            if ((*row)[REPORT_STATUS] != "Awaiting Verification") { jsonError(response, 400, "Report is not awaiting verification"); return; }
            (*row)[REPORT_STATUS] = "Resolved"; (*row)[RESOLVED_AT] = to_string(nowMs()); addHistory(*row, "Approved and archived", actor);
        } else {
            string reason = param(request, "reason");
            if ((*row)[REPORT_STATUS] != "Awaiting Verification" || reason.empty()) { jsonError(response, 400, "Report must await verification and a reason is required"); return; }
            (*row)[REPORT_STATUS] = "In Progress"; (*row)[REWORK_NOTE] = reason; (*row)[AFTER_IMAGE].clear(); (*row)[RESOLVED_AT].clear();
            addHistory(*row, "Rework requested: " + reason, actor);
        }
        touch(*row); Row updated = *row; store(rows); sendReport(response, updated);
    });
    server.Post("/api/reports/assign", [](const httplib::Request& request, httplib::Response& response) {
        int id = idFrom(param(request, "id")); string worker = param(request, "worker"), actor = param(request, "actor");
        auto rows = load(); Row* row = findReport(rows, id);
        if (!row) { jsonError(response, 404, "report not found"); return; }
        if (worker.empty() || ((*row)[REPORT_STATUS] != "Submitted" && (*row)[REPORT_STATUS] != "Assigned" && (*row)[REPORT_STATUS] != "Awaiting Verification")) { jsonError(response, 400, "Only unstarted reports can be assigned"); return; }
        string event = (*row)[REPORT_STATUS] == "Submitted" ? "Assigned to " + worker : "Reassigned to " + worker;
        (*row)[REPORT_STATUS] = "Assigned"; (*row)[ASSIGNED] = worker; touch(*row); addHistory(*row, event, actor);
        Row updated = *row; store(rows); sendReport(response, updated);
    });
    server.Post("/api/reports/delete-all", [](const httplib::Request&, httplib::Response& response) {
        auto rows = load();
        for (const auto& row : rows) images::deleteImage(idFrom(row[ID]));
        store({}); response.set_content("{\"deleted\":true}", "application/json");
    });
    server.Delete(R"(/api/reports/(CV-)?(\d+))", [](const httplib::Request& request, httplib::Response& response) {
        int id = idFrom(request.matches[2]); auto rows = load(); size_t before = rows.size();
        rows.erase(remove_if(rows.begin(), rows.end(), [&](const Row& row) { return idFrom(row[ID]) == id; }), rows.end());
        if (rows.size() == before) { jsonError(response, 404, "report not found"); return; }
        store(rows); images::deleteImage(id); response.set_content("{\"deleted\":true}", "application/json");
    });
    server.Post("/api/reports/verify", [](const httplib::Request& request, httplib::Response& response) {
        int id = idFrom(param(request, "id")); auto rows = load(); Row* row = findReport(rows, id);
        if (!row || ((*row)[REPORT_STATUS] != "Awaiting Verification" && (*row)[REPORT_STATUS] != "Submitted")) { jsonError(response, 404, "report not found or not awaiting verification"); return; }
        (*row)[REPORT_STATUS] = "Awaiting Verification"; addHistory(*row, "Report verified", param(request, "actor"));
        touch(*row); Row updated = *row; store(rows); sendReport(response, updated);
    });
    server.Post("/api/reports/complete", [](const httplib::Request& request, httplib::Response& response) {
        int id = idFrom(param(request, "id")); auto rows = load(); Row* row = findReport(rows, id);
        if (!row || (*row)[REPORT_STATUS] != "Assigned") { jsonError(response, 400, "Report must be assigned"); return; }
        (*row)[REPORT_STATUS] = "Resolved"; (*row)[RESOLVED_AT] = to_string(nowMs());
        addHistory(*row, "Cleanup completed", param(request, "actor")); touch(*row);
        Row updated = *row; store(rows); sendReport(response, updated);
    });
    server.Get(R"(/api/images/(\d+)(/(before|after))?)", [](const httplib::Request& request, httplib::Response& response) {
        int id = stoi(request.matches[1]); string variant = request.matches[3]; string path = images::getImagePath(id, variant);
        if (path.empty()) { response.status = 404; return; }
        ifstream file(path, ios::binary); stringstream bytes; bytes << file.rdbuf();
        auto ext = filesystem::path(path).extension().string();
        string mime = ext == ".png" ? "image/png" : ext == ".webp" ? "image/webp" : ext == ".gif" ? "image/gif" : "image/jpeg";
        response.set_content(bytes.str(), mime);
    });
    server.set_error_handler([](const auto&, auto& response) { if (response.body.empty()) jsonError(response, response.status ? response.status : 500, "request failed"); });
    cout << "CAMPuVISE API listening at http://127.0.0.1:8080\n";
    server.listen("127.0.0.1", 8080);
}
