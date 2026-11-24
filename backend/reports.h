#ifndef REPORTS_H
#define REPORTS_H

#include <string>
using namespace std;

struct Report
{
    int id;
    string location;
    string wasteType;
    string severity;
    string description;
    string status;
};

int getNextID();

void submitReport();
void viewReports();
void removeReport();

#endif
