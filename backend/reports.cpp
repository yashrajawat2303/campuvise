#include "reports.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int getNextID()
{
    int id = 1001;

    ifstream file("next_id.txt");

    if (file)
    {
        file >> id;
    }

    file.close();

    ofstream output("next_id.txt");
    output << id + 1;
    output.close();

    return id;
}

void submitReport()
{
    Report r;

    cout << "\n--- Submit Waste Report ---\n";

    cout << "Location: ";
    getline(cin, r.location);

    cout << "Waste type: ";
    getline(cin, r.wasteType);

    cout << "Severity (Low/Medium/High): ";
    getline(cin, r.severity);

    cout << "Description: ";
    getline(cin, r.description);

    r.status = "Pending";
    r.id = getNextID();

    ofstream file("reports.txt", ios::app);

    file << r.id << "|"
         << r.location << "|"
         << r.wasteType << "|"
         << r.severity << "|"
         << r.description << "|"
         << r.status << "\n";

    file.close();

    cout << "\nReport submitted successfully!\n";
    cout << "Report ID: " << r.id << "\n";
}

void viewReports()
{
    ifstream file("reports.txt");

    string line;

    cout << "\n--- Submitted Reports ---\n";

    if (!file)
    {
        cout << "No reports found.\n";
        return;
    }

    while (getline(file, line))
    {
        cout << line << "\n";
    }

    file.close();
}

void removeReport()
{
    int targetID;

    cout << "\nEnter Report ID to remove: ";
    cin >> targetID;
    cin.ignore();

    ifstream file("reports.txt");

    vector<string> reports;

    string line;
    bool found = false;

    while (getline(file, line))
    {
        int separator = line.find('|');

        if (separator == string::npos)
        {
            continue;
        }

        int reportID = stoi(line.substr(0, separator));

        if (reportID == targetID)
        {
            found = true;
        }
        else
        {
            reports.push_back(line);
        }
    }

    file.close();

    if (!found)
    {
        cout << "Report not found!\n";
        return;
    }

    ofstream output("reports.txt");

    for (string report : reports)
    {
        output << report << "\n";
    }

    output.close();

    cout << "Report removed successfully!\n";
}
