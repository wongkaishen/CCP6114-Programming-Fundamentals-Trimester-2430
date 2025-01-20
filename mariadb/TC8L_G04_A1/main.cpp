#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

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


int main() {
    ifstream inFile("fileinput2.txt");
    ofstream outFile("fileoutput2.txt");
    string line;
    vector<string> headers;
    vector<vector<string>> data;
    bool inCreateTable = false;
    string tableName;
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
        } else if (line.find("VALUES") != string::npos) {
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

            // UPDATE function
        else if (line.find("UPDATE") != string::npos) {
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

    }
    // Second pass: Process DELETE statements
    inFile.clear();
    inFile.seekg(0);
    while (getline(inFile, line)) {
        if (line.find("DELETE FROM") != string::npos) {
            processDelete(line, data, headers);
        }
    }

    // Write the table schema
    outFile << "CREATE TABLE " << tableName << "(" << endl;
    for (size_t i = 0; i < headers.size(); i++) {
        outFile << "  " << headers[i] << " TEXT";
        if (i < headers.size() - 1) {
            outFile << ",";
        }
        outFile << endl;
    }
    outFile << ");" << endl << endl;

    // Write the data rows
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
