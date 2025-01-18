#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

void processDelete(const string& line, vector<vector<string>>& data, const vector<string>& headers) {
    // Extract the conditions after WHERE
    size_t wherePos = line.find("WHERE");
    if (wherePos == string::npos) {
        cout << "Error: WHERE clause not found" << endl;
        return;
    }

    string conditions = line.substr(wherePos + 6);  // Skip "WHERE "
    conditions = conditions.substr(0, conditions.find(';')); // Remove semicolon

    // Split all conditions by OR
    vector<pair<string, string>> deleteConditions;  // Will store column-value pairs

    // Process the conditions string
    while (!conditions.empty()) {
        // Find next OR if it exists
        size_t orPos = conditions.find(" OR ");

        // Get single condition (either up to OR or the rest of string)
        string singleCondition = (orPos != string::npos) ?
            conditions.substr(0, orPos) : conditions;

        // Find equals sign in condition
        size_t equalsPos = singleCondition.find('=');
        if (equalsPos != string::npos) {
            string columnName = singleCondition.substr(0, equalsPos);
            string value = singleCondition.substr(equalsPos + 1);

            // Remove spaces
            columnName.erase(remove(columnName.begin(), columnName.end(), ' '), columnName.end());
            value.erase(remove(value.begin(), value.end(), ' '), value.end());

            deleteConditions.push_back({columnName, value});
        }

        // Move to next condition if OR exists
        if (orPos != string::npos) {
            conditions = conditions.substr(orPos + 4);  // Skip " OR "
        } else {
            conditions.clear();
        }
    }

    // Keep track of original size for deletion count
    size_t originalSize = data.size();

    // Remove rows that match any condition
    auto rowIter = data.begin();
    while (rowIter != data.end()) {
        bool shouldDelete = false;

        // Check each delete condition
        for (const auto& [columnName, value] : deleteConditions) {
            // Find column index
            int columnIndex = -1;
            for (int i = 0; i < headers.size(); i++) {
                if (headers[i] == columnName) {
                    columnIndex = i;
                    break;
                }
            }

            // Skip if column not found
            if (columnIndex == -1) {
                cout << "Column not found: " << columnName << endl;
                continue;
            }

            // Check if this row matches the condition
            if ((*rowIter)[columnIndex] == value) {
                shouldDelete = true;
                break;
            }
        }

        // Delete or move to next row
        if (shouldDelete) {
            rowIter = data.erase(rowIter);
        } else {
            ++rowIter;
        }
    }

    // Report results
    cout << "Deleted " << (originalSize - data.size()) << " rows" << endl;
}
int main() {
    ifstream inFile("fileinput1.txt");
    ofstream outFile("fileoutput1.txt");
    string line;
    vector<string> headers;
    vector<vector<string>> data;
    bool inCreateTable = false;
    string tableName;

    if (!inFile) {
        cout << "Error opening fileinput1.txt" << endl;
        return 1;
    }

    // First pass: Process everything except DELETE statements
    while (getline(inFile, line)) {
        // Skip DELETE statements in first pass
        if (line.find("DELETE FROM") != string::npos) {
            continue;
        }

        // Original code for CREATE TABLE and INSERT
        if (line.find("CREATE TABLE") != string::npos) {
            inCreateTable = true;
            size_t start = line.find("CREATE TABLE") + 12;
            size_t end = line.find("(");
            tableName = line.substr(start, end - start);
        }

        if (inCreateTable) {
            size_t start = line.find('(');
            size_t end = line.find(')');
            if (start != string::npos) {
                string columns = line.substr(start + 1);
                stringstream ss(columns);
                string column;

                while (getline(ss, column, ',')) {
                    size_t spacePos = column.find(' ');
                    if (spacePos != string::npos) {
                        headers.push_back(column.substr(0, spacePos));
                    }
                }
            }
            else if (end != string::npos) {
                stringstream ss(line);
                string column;
                while (getline(ss, column, ',')) {
                    size_t spacePos = column.find(' ');
                    if (spacePos != string::npos) {
                        headers.push_back(column.substr(0, spacePos));
                    }
                }
                inCreateTable = false;
            }
        }
        else if (line.find("VALUES") != string::npos) {
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
    }

    // Second pass: Process DELETE statements
    inFile.clear();
    inFile.seekg(0);
    while (getline(inFile, line)) {
        if (line.find("DELETE FROM") != string::npos) {
            processDelete(line, data, headers);
        }
    }

    // Write the CSV headers to terminal and file
    outFile << "CREATE TABLE " << tableName << "(" << endl;
    for (size_t i = 0; i < headers.size(); i++) {
        outFile << "  " << headers[i] << " TEXT";
        if (i < headers.size() - 1) {
            outFile << ",";
        }
        outFile << endl;
    }
    outFile << ");" << endl << endl;

    // Write the CSV header row
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

    // Write CSV rows (data) to terminal and file
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
