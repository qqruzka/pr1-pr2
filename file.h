#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "json.hpp"
#include <sstream>
#include "dbase.h"
using namespace std;
using json = nlohmann::json;
void initializePrimaryKey(dbase& db);
void lockPrimaryKey(dbase& db);
void unlockPrimaryKey(dbase& db);
void createDirectories(dbase& db, const json& structure);
void rewriteCSV(dbase& db, const string& table);
void loadSchema(dbase& db, const string& schema_file);
void saveSingleEntryToCSV(dbase& db, const string& table, const json& entry);
void updatePrimaryKey(dbase& db);
void handleClient(int client_socket, dbase& db);
