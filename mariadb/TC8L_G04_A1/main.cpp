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

void processCreateTable(const string& line, vector<string>& headers, string& accumulatedColumns, bool& inCreateTable, string& tableName) {
      if (line.find("CREATE TABLE") != string::npos) {
          // Extract table name
          size_t start = line.find("CREATE TABLE") + 12;
          size_t end = line.find("(");
          if (start != string::npos && end != string::npos) {
              tableName = line.substr(start, end - start);
              tableName.erase(remove(tableName.begin(), tableName.end(), ' '), tableName.end()); // Remove extra spaces
              inCreateTable = true; // Indicate that we're processing the table
          }
      }

      if (inCreateTable) {
          size_t start = line.find('(');
          size_t end = line.find(')');
          if (start != string::npos) {
              // Start of column definitions
              accumulatedColumns += line.substr(start + 1);
          } else {
              // Accumulate multi-line column definitions
              accumulatedColumns += line;
          }

          if (end != string::npos) {
              // End of column definitions
              accumulatedColumns = accumulatedColumns.substr(0, accumulatedColumns.find(')')); // Remove trailing `)`
              stringstream ss(accumulatedColumns);
              string column;
              while (getline(ss, column, ',')) {
                  size_t spacePos = column.find(' ');
                  if (spacePos != string::npos) {
                      headers.push_back(column.substr(0, spacePos)); // Extract column name
                  }
              }
              inCreateTable = false; // Done processing
              accumulatedColumns.clear(); // Clear accumulated data
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

void processUPDATE(const string& line, vector<string>& headers, vector<vector<string>>& data) {
    // Parse UPDATE statement
    size_t setPos = line.find("SET");     // Find where 'SET' starts in the line
    size_t wherePos = line.find("WHERE"); // Find where 'WHERE' starts in the line

    if (setPos != string::npos && wherePos != string::npos) {
        // Extract table name
        string tableName = line.substr(line.find("UPDATE") + 6, setPos - (line.find("UPDATE") + 6));
        tableName = tableName.substr(tableName.find_first_not_of(' '), tableName.find_last_not_of(' ') - tableName.find_first_not_of(' ') + 1);

        // Extract column name and value (from SET clause)
        string setClause = line.substr(setPos + 4, wherePos - (setPos + 4));
        size_t equalSetPos = setClause.find("="); // Find the equal sign to separate column name and value
        size_t startQuote = setClause.find("'", equalSetPos); // Start after the first quote
        size_t endQuote = setClause.find("'", startQuote + 1); // Find the end quote
        string updateColumn = setClause.substr(0, equalSetPos);
        updateColumn = updateColumn.substr(updateColumn.find_first_not_of(' '), updateColumn.find_last_not_of(' ') - updateColumn.find_first_not_of(' ') + 1);

        // Extract the update value
        string updateValue = setClause.substr(startQuote + 1, endQuote - startQuote - 1);

        // Extract column name and value (from WHERE clause)
        string whereClause = line.substr(wherePos + 5);
        size_t equalWherePos = whereClause.find("="); // Find the equal sign to separate column name and value
        string whereColumn = whereClause.substr(0, equalWherePos);
        whereColumn = whereColumn.substr(whereColumn.find_first_not_of(' '), whereColumn.find_last_not_of(' ') - whereColumn.find_first_not_of(' ') + 1);

        // Extract the WHERE value
        string whereValue = whereClause.substr(equalWherePos + 1);
        whereValue = whereValue.substr(whereValue.find_first_not_of(' '), whereValue.find_last_not_of(' ') - whereValue.find_first_not_of(' ') + 1);
        if (!whereValue.empty() && whereValue.back() == ';') {
            whereValue = whereValue.substr(0, whereValue.size() - 1);
        }

        // Find column indices for UPDATE and WHERE
        int updateIndex = -1, whereIndex = -1; // Initialize indices as not found (-1)
        for (size_t i = 0; i < headers.size(); i++) {
            if (headers[i] == updateColumn) updateIndex = i;
            if (headers[i] == whereColumn) whereIndex = i;
        }

        // Update the data in matching rows
        if (updateIndex != -1 && whereIndex != -1) { // Check if the indices are valid
            for (auto& row : data) {
                if (row[whereIndex] == whereValue) { // Compare the value in WHERE column
                    row[updateIndex] = updateValue; // Update the value if WHERE condition is matched
                }
            }
        } else {
            cerr << "Error: Column not found in the table headers.\n";
        }
    } else {
        cerr << "Error: Malformed UPDATE statement.\n";
    }
}


int main() {
    ifstream inFile("fileinput2.txt");
    ofstream outFile("fileoutput2.txt");
    string line;
    vector<string> headers;
    vector<vector<string>> data;
    bool inCreateTable = false;
    string accumulatedColumns;
    string tableName;

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
            processCreateTable(line, headers, accumulatedColumns, inCreateTable, tableName);
        } else if (line.find("VALUES") != string::npos) {
            processValues(line, data);
        }
        else if (line.find("UPDATE") != string::npos) {
        processUPDATE(line, headers, data);
        }
        else if (line.find("SELECT COUNT(*) FROM") != string::npos) {
        // Parse the SELECT COUNT(*) query
        size_t pos = line.find("FROM") + 5;
        string queryTable = line.substr(pos);
        queryTable.erase(remove(queryTable.begin(), queryTable.end(), ';'), queryTable.end());
        queryTable.erase(remove(queryTable.begin(), queryTable.end(), ' '), queryTable.end());

        // Match the table name and count rows
        if (queryTable == tableName) {
            cout << "Row count: " << data.size() << endl;
            outFile << "Row count: " << data.size() << endl;
        } else {
            cerr << "Error: Table '" << queryTable << "' not found." << endl;
            outFile << "Error: Table '" << queryTable << "' not found." << endl;
        }
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



