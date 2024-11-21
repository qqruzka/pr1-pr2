#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "json.hpp"
#include <sstream>
#include "dbase.h"
using namespace std;
using json = nlohmann::json;
void insert(dbase& db, const string& table, json entry);
void deleteRow(dbase& db, const string& column, const string& value, const string& table);
bool applyAndFilters(const json& entry, const Vector<MyPair<string, string>>& filters);
bool applyOrFilters(const json& entry, const Vector<MyPair<string, string>>& filters);
void select(dbase& db, const string& column, const string& column2, const string& from, Vector<MyPair<string, string>> filters, const string& filter_type);
void processQuery(const string& query, dbase& db);
void handleInsert(istringstream& iss, dbase& db, const string& table);
void logCommandToFile(const string& command, const string& filename);
void selectFromTable(dbase& db, const string& column, const string& table, const Vector<MyPair<string, string>>& filters, const string& filter_type);
void selectFromMultipleTables(dbase& db, const string& column1, const string& column2, const string& table1, const string& table2, Vector<MyPair<string, string>> filters, const string& filter_type);
