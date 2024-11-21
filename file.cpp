#include "dbase.h"
#include "file.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "json.hpp"
#include <sstream>
#include <mutex>
#include "query.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;
using json = nlohmann::json;
void initializePrimaryKey(dbase& db) {
    try {
        string pk_filename = db.schema_name + "/table_pk_sequence.txt";
        ifstream pk_file(pk_filename);
        
        if (pk_file) {
            pk_file >> db.current_pk;
        } else {
            db.current_pk = 0;
            ofstream pk_file_out(pk_filename);
            if (pk_file_out) {
                pk_file_out << db.current_pk << "\nunlocked";
            } else {
                throw runtime_error("Failed to create file: " + pk_filename);
            }
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void lockPrimaryKey(dbase& db) {
    try {
        string pk_filename = db.schema_name + "/table_pk_sequence.txt";
        ofstream pk_file(pk_filename);
        if (pk_file) {
            pk_file << db.current_pk << "\nlocked";
        } else {
            throw runtime_error("Failed to lock primary key file: " + pk_filename);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void unlockPrimaryKey(dbase& db) {
    try {
        string pk_filename = db.schema_name + "/table_pk_sequence.txt";
        ofstream pk_file(pk_filename);
        if (pk_file) {
            pk_file << db.current_pk << "\nunlocked";
        } else {
            throw runtime_error("Failed to unlock primary key file: " + pk_filename);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void createDirectories(dbase& db, const json& structure) {
    try {
        // Создаем директорию для схемы
        if (mkdir(db.schema_name.c_str(), 0777) && errno != EEXIST) {
            throw runtime_error("Failed to create directory: " + db.schema_name);
        }

        for (const auto& table : structure.items()) {
            string table_name = table.key();
            string table_path = db.schema_name + "/" + table_name;

            // Создаем директорию для таблицы
            if (mkdir(table_path.c_str(), 0777) && errno != EEXIST) {
                throw runtime_error("Failed to create directory: " + table_path);
            }
            db.filename = table_path + "/1.csv";

            ifstream check_file(db.filename);
            if (!check_file) {
                ofstream file(db.filename);
                if (file.is_open()) {
                    auto& columns = table.value();
                    for (size_t i = 0; i < columns.size(); ++i) {
                        file << columns[i].get<string>() << (i < columns.size() - 1 ? ", " : "");
                    }
                    file << "\n";
                    file.close();
                }
            }

            initializePrimaryKey(db);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void rewriteCSV(dbase& db, const string& table) {
    try {
        db.filename = db.schema_name + "/" + table + "/1.csv"; 
        ofstream file(db.filename); 

        if (file) {
            Node* table_node = db.findNode(table);
            if (table_node) {
                json columns = {"name", "age", "adress", "number"};

                for (const auto& column : columns) {
                    file << column.get<string>() << (column != columns.back() ? ", " : "");
                }
                file << "\n"; 

                for (size_t i = 0; i < table_node->data.getSize(); ++i) {
                    json entry = json::parse(table_node->data.get(i));
                    for (const auto& column : columns) {
                        file << entry[column.get<string>()].get<string>() << (column != columns.back() ? ", " : "");
                    }
                    file << "\n"; 
                }
            } else {
                throw runtime_error("Table not found: " + table);
            }
            file.close();
        } else {
            throw runtime_error("Failed to open data file for rewriting: " + db.filename);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void loadSchema(dbase& db, const string& schema_file) {
    try {
        ifstream file(schema_file);
        if (file) {
            json schema;
            file >> schema;
            db.schema_name = schema["name"];
            createDirectories(db, schema["structure"]);
            for (const auto& table : schema["structure"].items()) {
                db.addNode(table.key());
            }
        } else {
            throw runtime_error("Failed to open schema file.");
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void saveSingleEntryToCSV(dbase& db, const string& table, const json& entry) {
    try {
        string filename = db.schema_name + "/" + table + "/1.csv"; 
        ofstream file(filename, ios::app);
        
        if (file) {
            if (entry.contains("name") && entry.contains("age") && entry.contains("adress") && entry.contains("number")) {
                // Записываем значения без кавычек
                file << entry["name"].get<string>() << ", "<< entry["age"] << ", "<< entry["adress"] << ", "<< entry["number"] << "\n";
                cout << "Data successfully saved for: " << entry.dump() << endl;
            } else {
                throw runtime_error("Missing required fields in entry.");
            }
        } else {
            throw runtime_error("Failed to open data file for saving: " + filename);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
void updatePrimaryKey(dbase& db) {
    try {
        string pk_filename = db.schema_name + "/table_pk_sequence.txt";

        ifstream pk_file(pk_filename);
        if (pk_file) {
            pk_file >> db.current_pk;
        } else {
            db.current_pk = 0;
        }
        pk_file.close();

        db.current_pk++;

        ofstream pk_file_out(pk_filename);
        if (pk_file_out) {
            pk_file_out << db.current_pk << "\nlocked";
        } else {
            throw runtime_error("Failed to open file for updating: " + pk_filename);
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
mutex db_mutex;
void handleClient(int client_socket, dbase& db) {
    char buffer[1024];
    
    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            // Если чтение завершилось, клиент закрыл соединение
            break;
        }

        buffer[bytes_read] = '\0'; // Завершаем строку
        string query(buffer);
        logCommandToFile(query, "command_history.txt");

        string response; // Строка для хранения ответа

        try {
            istringstream iss(query);
            string action;
            iss >> action;

            // Блокировка для безопасного доступа к базе данных
            lock_guard<mutex> lock(db_mutex);

            if (action == "INSERT") {
                string table;
                iss >> table;
                handleInsert(iss, db, table);
                response = "Insert successful.";
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
                    response = "Select executed.";
                } else {
                    // Если нет условий WHERE, просто выполняем выборку
                    select(db, column1, column2, table1 + " " + table2, {}, "AND");
                    response = "Select executed without conditions.";
                }
            } else if (action == "DELETE") {
                string table, column, value;
                iss >> table >> column >> value;
                deleteRow(db, column, value, table);
                response = "Delete successful.";
            } else {
                throw runtime_error("Unknown command: " + query);
            }
        } catch (const exception& e) {
            response = "Error: " + string(e.what());
        }

        // Отправляем ответ клиенту
        send(client_socket, response.c_str(), response.size(), 0);
    }

    close(client_socket);
}
