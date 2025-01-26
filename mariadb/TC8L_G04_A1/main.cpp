// *********************************************************
// Program: main.cpp
// Course: CCP6114 Programming Fundamentals
// Lecture Class: TC8L
// Tutorial Class: T16L
// Trimester: 2430
// Member_1: 243UC247DH | WONG KAI SHEN | wong.kai.shen@student.mmu.edu.my | 0167129682
// Member_2: 243UC2467K | Teh Shin Rou | teh.shin.rou@student.mmu.edu.my | 0102407909
// Member_3: 243UC2466T | Nyiam Zi Qin | NYIAM.ZI.QIN@student.mmu.edu.my | 0189700993
// Member_4: 243UC246NQ | Yen Ming Jun | yen.ming.jun@student.mmu.edu.my | 01153726266
// *********************************************************
// Task Distribution
// Member_1: ProcessCreateFile,processTables,ProcessCreateTable, ProcessValues, ProcessSelectQuary,processDatabases, main
// Member_2: ProcessUpdate, Update flowchart,
// Member_3: processCount, Count Flowchart,
// Member_4: processDelete, Delete Flowchart,
// *********************************************************
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

void processCreateFile(const string& line, ofstream& outFile) {
    size_t start = line.find("CREATE") + 7; // Find "CREATE" and move past it
    size_t end = line.find(";");            // Find the semicolon
    if (start != string::npos && end != string::npos) {
        string fileName = line.substr(start, end - start);
        fileName.erase(remove(fileName.begin(), fileName.end(), ' '), fileName.end()); // Remove spaces

        // Close the current output file and open the new one
        outFile.close();
        outFile.open(fileName);
        if (!outFile) {
            cerr << "Error: Unable to create file " << fileName << endl;
            return;
        }
    }
}

void processTables(ofstream& outFile, const string& tableName) {
    cout << tableName << endl;
    outFile << tableName << endl;
}

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
    if (line.find("VALUES") != string::npos) {
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

}

void processDatabases(const string& line, const string& filein) {
    // Check if the line contains "DATABASES;"
    if (line.find("DATABASES;") != string::npos) {
        cout << "C:\\mariadb\\" << filein << endl; // Output the required message
    }
}

void processUPDATE(const string& line, vector<string>& headers, vector<vector<string>>& data) {
    // Parse UPDATE statement
    size_t setPos = line.find("SET");     // Find where 'SET' starts in the line
    size_t wherePos = line.find("WHERE"); // Find where 'WHERE' starts in the line

    if (setPos != string::npos && wherePos != string::npos) {
        // Extract table name
        string tableName = line.substr(line.find("UPDATE") + 6, setPos - (line.find("UPDATE") + 6));
        tableName = tableName.substr(tableName.find_first_not_of(' '), tableName.find_last_not_of(' ') - tableName.find_first_not_of(' ') + 1);

        // Extract column name and value (from SET)
        string setClause = line.substr(setPos + 4, wherePos - (setPos + 4));
        size_t equalSetPos = setClause.find("="); // Find the equal sign to separate column name and value
        size_t startQuote = setClause.find("'", equalSetPos); // Start after the first quote
        size_t endQuote = setClause.find("'", startQuote + 1); // Find the end quote
        string updateColumn = setClause.substr(0, equalSetPos);
        updateColumn = updateColumn.substr(updateColumn.find_first_not_of(' '), updateColumn.find_last_not_of(' ') - updateColumn.find_first_not_of(' ') + 1);

        // Extract the update value
        string updateValue = setClause.substr(startQuote + 1, endQuote - startQuote - 1);

        // Extract the WHERE value (from WHERE)
        string whereClause = line.substr(wherePos + 5);
        size_t equalWherePos = whereClause.find("="); // Find the equal sign to separate column name and value
        string whereColumn = whereClause.substr(0, equalWherePos);
        whereColumn = whereColumn.substr(whereColumn.find_first_not_of(' '), whereColumn.find_last_not_of(' ') - whereColumn.find_first_not_of(' ') + 1);

        string whereValue = whereClause.substr(equalWherePos + 1);
        whereValue = whereValue.substr(whereValue.find_first_not_of(' '), whereValue.find_last_not_of(' ') - whereValue.find_first_not_of(' ') + 1);

        //Remove trailing semicolon from WhereValue if it exists
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

void processCount(const string &line, const string &tableName, const vector<vector<string>> &data, ofstream &outFile) {
    // Parse the SELECT COUNT(*) query
    if (line.find("SELECT COUNT(*)") != string::npos){
    size_t pos = line.find("FROM") + 5;
    string queryTable = line.substr(pos);
    queryTable.erase(remove(queryTable.begin(), queryTable.end(), ';'), queryTable.end());
    queryTable.erase(remove(queryTable.begin(), queryTable.end(), ' '), queryTable.end());

    // Match the table name and count rows
    if (queryTable == tableName) {
        cout << data.size() << endl;
        outFile << data.size() << endl;
    } else {
        cerr << "Error: Table '" << queryTable << "' not found." << endl;
        outFile << "Error: Table '" << queryTable << "' not found." << endl;
        }
    }
}

void processSelectQuery(const string& line, const vector<string>& headers, const vector<vector<string>>& data, ofstream& outFile, string& tableName) {
    if (line.find("SELECT * FROM") != string::npos) {
        // Handle SELECT * FROM queries
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
    }
}

int main() {
    string filein = "fileInput1.mdb";// <=== Please change the file input here
    ifstream inFile(filein);
    ofstream outFile;
    string line;
    vector<string> headers;
    vector<vector<string>> data;
    bool inCreateTable = false;
    string accumulatedColumns;
    string tableName;

    if (!inFile) {
        cout << "Please check if the file exist in your directory!" << endl;
        return 1;
    }

    while (getline(inFile, line)) {
        if (line.find("CREATE ") != string::npos && line.find("TABLE") == string::npos) {
            processCreateFile(line, outFile);
        }   else if (line.find("CREATE TABLE") != string::npos || inCreateTable) {
            //Process CREATE TABLE
            processCreateTable(line, headers, accumulatedColumns, inCreateTable, tableName);
        } else if (line.find("VALUES") != string::npos) {
            //Process INSERT VALUE
            processValues(line, data);
        } else if (line.find("UPDATE") != string::npos) {
            //Process UPDATE statements
            processUPDATE(line, headers, data);
        } else if (line.find("DELETE FROM") != string::npos) {
            //Process DELETE statements
            processDelete(line, data, headers);
        } else if (line.find("DATABASES;") != string::npos) {
            processDatabases(line, filein);
        } else if (line.find("SELECT COUNT(*) FROM") != string::npos) {
        // Handle SELECT COUNT(*) FROM queries
            processCount(line, tableName, data, outFile);
        } else if(line.find("SELECT * FROM") != string::npos) {
        //Process Select From statements
            processSelectQuery(line, headers, data, outFile, tableName);
        }
    }
    // Output TABLES
    processTables(outFile, tableName);


    inFile.close();
    outFile.close();
    return 0;
}



