#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "cleanup.h"

using namespace std;

// Splits one report into its individual pieces
vector<string> splitReport(string line)
{
    vector<string> parts;
    string part;
    stringstream ss(line);

    while (getline(ss, part, '|'))
    {
        parts.push_back(part);
    }

    return parts;
}

// Puts the pieces back into one line
string joinReport(vector<string> parts)
{
    string line;

    for (int i = 0; i < parts.size(); i++)
    {
        line += parts[i];

        if (i < parts.size() - 1)
        {
            line += "|";
        }
    }

    return line;
}

void assignWorker()
{
    int targetID;
    string workerName;

    cout << "\nEnter Report ID: ";
    cin >> targetID;
    cin.ignore();

    cout << "Enter worker name: ";
    getline(cin, workerName);

    ifstream file("reports.txt");

    vector<string> reports;
    string line;
    bool found = false;

    while (getline(file, line))
    {
        vector<string> parts = splitReport(line);

        if (parts.size() < 6)
        {
            reports.push_back(line);
            continue;
        }

        int reportID = stoi(parts[0]);

        if (reportID == targetID)
        {
            found = true;

            if (parts[5] != "Pending")
            {
                cout << "Report is not Pending!\n";
                file.close();
                return;
            }

            parts[5] = "Assigned";

            // Worker name goes at the end of the report
            if (parts.size() >= 7)
            {
                parts[6] = workerName;
            }
            else
            {
                parts.push_back(workerName);
            }

            line = joinReport(parts);
        }

        reports.push_back(line);
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

    cout << "Worker assigned successfully!\n";
}

void completeCleanup()
{
    int targetID;

    cout << "\nEnter Report ID: ";
    cin >> targetID;
    cin.ignore();

    ifstream file("reports.txt");

    vector<string> reports;
    string line;
    bool found = false;

    while (getline(file, line))
    {
        vector<string> parts = splitReport(line);

        if (parts.size() < 6)
        {
            reports.push_back(line);
            continue;
        }

        int reportID = stoi(parts[0]);

        if (reportID == targetID)
        {
            found = true;

            if (parts[5] != "Assigned")
            {
                cout << "Report must be Assigned first!\n";
                file.close();
                return;
            }

            parts[5] = "Completed";

            line = joinReport(parts);
        }

        reports.push_back(line);
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

    cout << "Cleanup completed successfully!\n";
}
