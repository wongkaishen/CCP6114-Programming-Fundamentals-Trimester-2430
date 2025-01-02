#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

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

    while (getline(inFile, line)) {
        // Detect the start of a CREATE TABLE block
        if (line.find("CREATE TABLE") != string::npos) {
            inCreateTable = true;
            size_t start = line.find("CREATE TABLE") + 12;
            size_t end = line.find("(");
            tableName = line.substr(start, end - start);  // Extract table name
        }

        // Parse headers within the CREATE TABLE block
        if (inCreateTable) {
            size_t start = line.find('(');
            size_t end = line.find(')');
            if (start != string::npos) {
                string columns = line.substr(start + 1);  // Skip the '('
                stringstream ss(columns);
                string column;

                // Process columns from the current line
                while (getline(ss, column, ',')) {
                    // Remove spaces from the column name and data type
                    size_t spacePos = column.find(' ');
                    if (spacePos != string::npos) {
                        // Extract only the column name
                        headers.push_back(column.substr(0, spacePos));
                    }
                }
            }
            // If line contains the closing parenthesis, finish parsing the table schema
            else if (end != string::npos) {
                stringstream ss(line);
                string column;
                while (getline(ss, column, ',')) {
                    size_t spacePos = column.find(' ');
                    if (spacePos != string::npos) {
                        // Extract only the column name
                        headers.push_back(column.substr(0, spacePos));
                    }
                }
                inCreateTable = false; // End of CREATE TABLE block
            }
        }
        else if (line.find("VALUES") != string::npos) {
            // Parse the VALUES line to extract data
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
