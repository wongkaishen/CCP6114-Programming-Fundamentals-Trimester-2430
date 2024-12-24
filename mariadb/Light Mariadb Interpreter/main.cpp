#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class DatabaseManager {
private:
    vector<string> databases;
    const string filename = "fileinput1.txt";

    void loadDatabasesFromFile() {
        ifstream infile(filename);
        string dbName;
        while (getline(infile, dbName)) {
            databases.push_back(dbName);
        }
        infile.close();
    }

    void saveDatabasesToFile() {
        ofstream outfile(filename, ios::trunc);
        for (const auto& dbName : databases) {
            outfile << dbName << endl;
        }
        outfile.close();
    }

public:
    DatabaseManager() {
        loadDatabasesFromFile();
    }

    void createDatabase(const string& dbName) {
        if (find(databases.begin(), databases.end(), dbName) != databases.end()) {
            cout << "Database '" << dbName << "' already exists." << endl;
            return;
        }
        databases.push_back(dbName);
        saveDatabasesToFile();
        cout << "Database '" << dbName << "' created successfully." << endl;
    }

    void viewDatabases() {
        if (databases.empty()) {
            cout << "No databases found." << endl;
            return;
        }
        cout << "Databases:" << endl;
        for (const auto& dbName : databases) {
            cout << "- " << dbName << endl;
        }
    }
};

int main() {
    DatabaseManager dbManager;
    int choice;
    string dbName;

    do {
        cout << "\nDatabase Management System" << endl;
        cout << "1. Create Database" << endl;
        cout << "2. View Databases" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter database name: ";
                cin >> dbName;
                dbManager.createDatabase(dbName);
                break;
            case 2:
                dbManager.viewDatabases();
                break;
            case 0:
                cout << "Exiting..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);

    return 0;
}
