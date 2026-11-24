#include <iostream>
#include "reports.h"
#include "cleanup.h"

using namespace std;

int main()
{
    int choice;

    while (true)
    {
        cout << "\n===== CAMPuVISE BACKEND =====\n";
        cout << "1. Submit report\n";
        cout << "2. View reports\n";
        cout << "3. Remove report\n";
        cout << "4. Assign worker\n";
        cout << "5. Complete cleanup\n";
        cout << "6. Exit\n";
        cout << "Choose: ";

        cin >> choice;
        cin.ignore();

        if (choice == 1)
        {
            submitReport();
        }
        else if (choice == 2)
        {
            viewReports();
        }
        else if (choice == 3)
        {
            removeReport();
        }
        else if (choice == 4)
        {
            assignWorker();
        }
        else if (choice == 5)
        {
            completeCleanup();
        }
        else if (choice == 6)
        {
            cout << "Backend shutting down...\n";
            break;
        }
        else
        {
            cout << "Invalid choice!\n";
        }
    }

    return 0;
}
