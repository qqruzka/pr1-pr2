#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <stdexcept>
#include "pair.h"
#include "json.hpp"
#include "vector.h"
using namespace std;
using json = nlohmann::json;
struct Node {
    string name;
    Vector<string> data;
    Node* next;

    Node(const string& name) : name(name), next(nullptr) {}
};

struct dbase {
    string filename; 
    string schema_name;
    Node* head;
    int current_pk;

    dbase() : head(nullptr), current_pk(0) {}

    ~dbase() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    Node* findNode(const string& table_name) {
        Node* current = head;
        while (current) {
            if (current->name == table_name) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    void addNode(const string& table_name) {
        Node* new_node = new Node(table_name);
        new_node->next = head;
        head = new_node;
    }

    size_t getColumnCount(const string& table) {
        Node* table_node = findNode(table);
        if (table_node) {
            string filename = schema_name + "/" + table + "/1.csv";
            ifstream file(filename);
            if (file) {
                string header;
                if (getline(file, header)) {
                    size_t comma_count = std::count(header.begin(), header.end(), ',');
                    return comma_count + 1;
                }
            }
        }
        return 0; 
    }

    void load() {
        Node* current = head;
        while (current) {
            try {
                filename = schema_name + "/" + current->name + "/1.csv"; 
                ifstream file(filename);
                if (file) {
                    string line;
                    bool is_header = true;
                    while (getline(file, line)) {
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);

    if (is_header) {
        is_header = false;
        continue;
    }

    istringstream iss(line);
    Vector<string> fields;
    string field;

    while (getline(iss, field, ',')) {
        field.erase(0, field.find_first_not_of(" \t"));
        field.erase(field.find_last_not_of(" \t") + 1);
        if (!field.empty()) {
            fields.addEnd(field);
        }
    }

    if (fields.getSize() == 4) {
        string entry = fields.get(0) + ", " + fields.get(1) + ", " + fields.get(2) + ", " + fields.get(3);
        current->data.addEnd(entry);
    }
}
                } else {
                    throw runtime_error("Failed to open data file: " + filename);
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            current = current->next;
        }
    }
};
