#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "json.hpp"
#include <sstream>
#include "dbase.h"
#include "file.h"
#include "vector.h"
#include "query.h"
#include <unistd.h>
using namespace std;
using json = nlohmann::json;
void selectFromMultipleTables(dbase& db, const string& column1, const string& column2, const string& table1, const string& table2, Vector<MyPair<string, string>> filters, const string& filter_type) {
    Node* table_node1 = db.findNode(table1);
    Node* table_node2 = db.findNode(table2);

    if (!table_node1 || !table_node2) {
        cout << "One or both tables not found: " << table1 << ", " << table2 << endl;
        return;
    }

    bool data_found = false;

    for (size_t i = 0; i < table_node1->data.getSize(); ++i) {
        json entry1 = json::parse(table_node1->data.get(i));

        for (size_t j = 0; j < table_node2->data.getSize(); ++j) {
            json entry2 = json::parse(table_node2->data.get(j));

            bool valid_row1 = (filter_type == "AND") ? applyAndFilters(entry1, filters) : applyOrFilters(entry1, filters);
            bool valid_row2 = (filter_type == "AND") ? applyAndFilters(entry2, filters) : applyOrFilters(entry2, filters);

            if (valid_row1 && valid_row2) {
                if (entry1.contains(column1) && entry2.contains(column2)) {
                    cout << entry1[column1].get<string>() << ", " << entry2[column2].get<string>() << endl;
                    data_found = true;
                }
            }
        }
    }

    if (!data_found) {
        cout << "No data found in the cross join of " << table1 << " and " << table2 << endl;
    }
}

void insert(dbase& db, const string& table, json entry) {
    Node* table_node = db.findNode(table);
    if (table_node) {
        updatePrimaryKey(db); 
        entry["id"] = db.current_pk; 

        table_node->data.addEnd(entry.dump());
        cout << "Inserted: " << entry.dump() << endl;

        saveSingleEntryToCSV(db, table, entry);
    } else {
        cout << "Table not found: " << table << endl;
    }
}
void deleteRow(dbase& db, const string& column, const string& value, const string& table) {
    Node* table_node = db.findNode(table);
    if (table_node) {
        Vector<string> new_data;
        bool found = false;

        for (size_t i = 0; i < table_node->data.getSize(); ++i) {
            json entry = json::parse(table_node->data.get(i));
            if (entry.contains(column) && entry[column].get<string>() == value) {
                found = true;
                cout << "Deleted row: " << entry.dump() << endl;
            } else {
                new_data.addEnd(table_node->data.get(i));
            }
        }

        if (found) {
            table_node->data = new_data;
            rewriteCSV(db, table);
        } else {
            cout << "Row with " << column << " = " << value << " not found in table " << table << endl;
        }
    } else {
        cout << "Table not found: " << table << endl;
    }
}
bool applyAndFilters(const json& entry, const Vector<MyPair<string, string>>& filters) {
    for (size_t i = 0; i < filters.getSize(); ++i) {
        const string& filter_column = filters.get(i).first;
        const string& filter_value = filters.get(i).second;

        if (!entry.contains(filter_column) || entry[filter_column].get<string>() != filter_value) {
            return false;
        }
    }
    return true;
}

bool applyOrFilters(const json& entry, const Vector<MyPair<string, string>>& filters) {
    for (size_t i = 0; i < filters.getSize(); ++i) {
        const string& filter_column = filters.get(i).first;
        const string& filter_value = filters.get(i).second;

        if (entry.contains(filter_column) && entry[filter_column].get<string>() == filter_value) {
            return true;
        }
    }
    return false;
}

void selectFromTable(dbase& db, const string& column, const string& table, const Vector<MyPair<string, string>>& filters, const string& filter_type) {
    Node* table_node = db.findNode(table);
    
    if (table_node) {
        bool data_found = false;
        for (size_t i = 0; i < table_node->data.getSize(); ++i) {
            json entry = json::parse(table_node->data.get(i));
            bool valid_row = (filter_type == "AND") ? applyAndFilters(entry, filters) : applyOrFilters(entry, filters);

            if (valid_row) {
                data_found = true;
                cout << entry[column].get<string>() << endl;
            }
        }
        if (!data_found) {
            cout << "No matching data found in the table." << endl;
        }
    } else {
        cout << "Table not found: " << table << endl;
    }
}
void select(dbase& db, const string& column, const string& column2, const string& from, Vector<MyPair<string, string>> filters, const string& filter_type) {
    istringstream iss(from);
    string table1, table2;

    iss >> table1; // Получаем имя первой таблицы
    if (iss >> table2) { // Получаем имя второй таблицы, если оно есть
        // Если есть второй столбец, то выполняем выборку из нескольких таблиц
        selectFromMultipleTables(db, column, column2, table1, table2, filters, filter_type);
    } else {
        // Выполняем выборку из одной таблицы
        selectFromTable(db, column, table1, filters, filter_type);
    }
}
void logCommandToFile(const string& command, const string& filename) {
    ofstream log_file(filename, ios::app);
    if (log_file) {
        log_file << command << endl;
    } else {
        cout << "Error: Unable to open log file." << endl;
    }
}
void handleInsert(istringstream& iss, dbase& db, const string& table) {
    Vector<string> args;
    string arg;
    while (iss >> arg) {
        args.addEnd(arg);
    }

    size_t expected_arg_count = db.getColumnCount(table);
    if (args.getSize() > expected_arg_count) {
        cout << "Error: Too many arguments (" << args.getSize() << ") for INSERT command." << endl;
        return;
    } else if (args.getSize() < expected_arg_count) {
        cout << "Error: Not enough arguments (" << args.getSize() << ") for INSERT command." << endl;
        return;
    }

    json entry;
    entry["name"] = args.get(0);
    entry["age"] = args.get(1);
    entry["adress"] = args.get(2);
    entry["number"] = args.get(3);

    insert(db, table, entry);
}

void processQuery(const string& query, dbase& db) {
    istringstream iss(query);
    string action;
    iss >> action;

    if (action == "INSERT") {
        string table;
        iss >> table;
        handleInsert(iss, db, table);
    } else if (action == "SELECT") {
        string from, column1, and_word, column2, table1, table2, where, filter_column1, equals1, filter_value1;
        string filter_type, filter_column2, equals2, filter_value2;
        iss >> from >> table1 >> column1 >> and_word >> table2 >> column2 >> where;

        Vector<MyPair<string, string>> filters;

        if (where == "WHERE") {
            iss >> filter_column1 >> equals1 >> filter_value1;
            filters.addEnd(MyPair<string, string>(filter_column1, filter_value1));

            if (iss >> filter_type >> filter_column2 >> equals2 >> filter_value2) {
                // Добавляем второй фильтр
                filters.addEnd(MyPair<string, string>(filter_column2, filter_value2));
            } else {
                filter_type = "AND"; // По умолчанию используем AND
            }

            select(db, column1, column2, table1 + " " + table2, filters, filter_type);
        } else {
            // Если нет условий WHERE, просто выполняем выборку
            select(db, column1, column2, table1 + " " + table2, {}, "AND");
        }
    } else if (action == "DELETE") {
        string table, column, value;
        iss >> table >> column >> value;
        deleteRow(db, column, value, table);
    } else {
        throw runtime_error("Unknown command: " + query);
    }
}
