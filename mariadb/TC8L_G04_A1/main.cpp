// *********************************************************
// Program: YOUR_FILENAME.cpp
// Course: CCP6114 Programming Fundamentals
// Lecture Class: TC3L
// Tutorial Class: TT5L
// Trimester: 2430
// Member_1: 243UC247DH | WONG KAI SHEN | wong.kai.shen@student.mmu.edu.my | 0167129682
// Member_2: ID | NAME | EMAIL | PHONE
// Member_3: ID | NAME | EMAIL | PHONE
// Member_4: ID | NAME | EMAIL | PHONE
// *********************************************************
// Task Distribution
// Member_1: Create Database,View Database,CREATE TABLE, CSV export,
// Member_2:
// Member_3:
// Member_4:
// *********************************************************
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

void processCreateTable(const string& line, vector<string>& headers, string& accumulatedColumns, bool& inCreateTable) {
    size_t start = line.find("CREATE TABLE") + 12;
    size_t end = line.find("(");
    if (start != string::npos && end != string::npos) {
        inCreateTable = true;
    }

    if (inCreateTable) {
        start = line.find('(');
        end = line.find(')');
        if (start != string::npos) {
            // Start of column definitions
            accumulatedColumns += line.substr(start + 1);
        } else {
            accumulatedColumns += line; // Accumulate multi-line definitions
        }

        if (end != string::npos) {
            // End of column definitions
            accumulatedColumns = accumulatedColumns.substr(0, accumulatedColumns.find(')')); // Remove trailing `)`
            stringstream ss(accumulatedColumns);
            string column;
            while (getline(ss, column, ',')) {
                size_t spacePos = column.find(' ');
                if (spacePos != string::npos) {
                    headers.push_back(column.substr(0, spacePos));
                }
            }
            inCreateTable = false;
            accumulatedColumns.clear();
        }
    }
}

void processValues(const string& line, vector<vector<string>>& data) {
    size_t start = line.find('(') + 1;
    size_t end = line.find(')');
    string values = line.substr(start, end - start);

    vector<string> row;
    string value;
    bool inQuote = false;

    for (char c : values) {
        if (c == '\'') {
            inQuote = !inQuote;
        } else if (c == ',' && !inQuote) {
            if (!value.empty()) {
                row.push_back(value);
                value.clear();
            }
        } else if (inQuote || (!inQuote && (isdigit(c) || isalpha(c) || c == ' '))) {
            value += c;
        }
    }
    if (!value.empty()) {
        row.push_back(value);
    }
    data.push_back(row);
}

void processDelete(const string& line, vector<vector<string>>& data, const vector<string>& headers) {
    size_t wherePos = line.find("WHERE");
    if (wherePos == string::npos) {
        cout << "Error: WHERE clause not found" << endl;
        return;
    }

    string conditions = line.substr(wherePos + 6);  // Skip "WHERE "
    conditions = conditions.substr(0, conditions.find(';')); // Remove semicolon

    vector<pair<string, string>> deleteConditions;  // Column-value pairs

    while (!conditions.empty()) {
        size_t orPos = conditions.find(" OR ");
        string singleCondition = (orPos != string::npos) ? conditions.substr(0, orPos) : conditions;

        size_t equalsPos = singleCondition.find('=');
        if (equalsPos != string::npos) {
            string columnName = singleCondition.substr(0, equalsPos);
            string value = singleCondition.substr(equalsPos + 1);

            columnName.erase(remove(columnName.begin(), columnName.end(), ' '), columnName.end());
            value.erase(remove(value.begin(), value.end(), ' '), value.end());

            deleteConditions.push_back({columnName, value});
        }

        if (orPos != string::npos) {
            conditions = conditions.substr(orPos + 4);  // Skip " OR "
        } else {
            conditions.clear();
        }
    }

    size_t originalSize = data.size();

    auto rowIter = data.begin();
    while (rowIter != data.end()) {
        bool shouldDelete = false;

        for (const auto& [columnName, value] : deleteConditions) {
            int columnIndex = -1;
            for (int i = 0; i < headers.size(); i++) {
                if (headers[i] == columnName) {
                    columnIndex = i;
                    break;
                }
            }

            if (columnIndex == -1) {
                cout << "Column not found: " << columnName << endl;
                continue;
            }

            if ((*rowIter)[columnIndex] == value) {
                shouldDelete = true;
                break;
            }
        }

        if (shouldDelete) {
            rowIter = data.erase(rowIter);
        } else {
            ++rowIter;
        }
    }

    cout << "Deleted " << (originalSize - data.size()) << " rows" << endl;
}



int main() {
    ifstream inFile("fileinput2.txt");
    ofstream outFile("fileoutput2.txt");
    string line;
    vector<string> headers;
    vector<vector<string>> data;
    bool inCreateTable = false;
    string accumulatedColumns;

    if (!inFile) {
        cout << "Error opening fileinput2.txt" << endl;
        return 1;
    }

    // First pass: Process CREATE TABLE and INSERT
    while (getline(inFile, line)) {
        if (line.find("DELETE FROM") != string::npos) {
            continue;
        }

        if (line.find("CREATE TABLE") != string::npos || inCreateTable) {
            processCreateTable(line, headers, accumulatedColumns, inCreateTable);
        } else if (line.find("VALUES") != string::npos) {
            processValues(line, data);
        }
    }

    // Second pass: Process DELETE statements
    inFile.clear();
    inFile.seekg(0);
    while (getline(inFile, line)) {
        if (line.find("DELETE FROM") != string::npos) {
            processDelete(line, data, headers);
        }
    }

    // Write the headers
    for (size_t i = 0; i < headers.size(); i++) {
        cout << headers[i];
        outFile << headers[i];
        if (i < headers.size() - 1) {
            cout << ",";
            outFile << ",";
        }
    }
    cout << endl;
    outFile << endl;

    // Write the data rows
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); i++) {
            cout << row[i];
            outFile << row[i];
            if (i < row.size() - 1) {
                cout << ",";
                outFile << ",";
            }
        }
        cout << endl;
        outFile << endl;
    }

    inFile.close();
    outFile.close();
    return 0;
}



