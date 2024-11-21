#ifndef HEADER_STACK_HPP
#define HEADER_STACK_HPP

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Класс узла стека
class NodeSt {
public:
    string value; // Значение узла
    NodeSt* next; // Указатель на следующий узел

    // Конструктор узла
    NodeSt(const string& val) : value(val), next(nullptr) {}
};

// Класс стека
class Stack {
private:
    NodeSt* top; // Указатель на верхний элемент стека

public:
    // Конструктор стека
    Stack() : top(nullptr) {} // Инициализация указателя на верхний элемент

    // Деструктор для освобождения памяти
    ~Stack() {
        while (top) {
            pop(); // Освобождение памяти при разрушении стека
        }
    }

    // Функция для добавления элемента в стек
    void push(const string& value) {
        NodeSt* newNodeSt = new NodeSt(value); // Создание нового узла
        newNodeSt->next = top; // Присоединение нового узла к вершине стека
        top = newNodeSt; // Обновление указателя на верхний элемент
    }

    // Функция для удаления элемента из стека
    void pop() {
        if (top == nullptr) { // Если стек пустой
            cout << "Stack is empty, nothing to remove." << endl; // Сообщение об ошибке
            return;
        }
        NodeSt* temp = top; // Сохранение верхнего узла
        top = top->next; // Перемещение указателя на следующий узел
        delete temp; // Освобождение памяти
    }

    // Функция для чтения элементов стека
    void read() const {
        if (top == nullptr) { // Если стек пустой
            cout << "Stack is empty." << endl; // Сообщение об ошибке
            return;
        }
        NodeSt* temp = top; // Указатель на верхний узел
        while (temp) {
            cout << temp->value << " -> "; // Вывод значения
            temp = temp->next; // Переход к следующему узлу
        }
        cout << "NULL" << endl; // Завершение вывода
    }

    // Функция для записи состояния стека в файл
    void writeToFile(const string& filename, const string& sourceFile) {
        ifstream checkFile(filename); // Проверка существования файла
        if (checkFile.peek() == ifstream::traits_type::eof()) { // Если файл пустой
            loadFromFile(sourceFile); // Загрузка из исходного файла
        }
        checkFile.close(); // Закрытие файла
        ofstream file(filename, ios::trunc); // Открытие файла для записи с обнулением
        NodeSt* temp = top; // Указатель на верхний узел
        while (temp) {
            file << temp->value; // Запись значения в файл
            if (temp->next) {
                file << " -> "; // Разделитель
            }
            temp = temp->next; // Переход к следующему узлу
        }
        file << " -> NULL\n"; // Завершение записи
        file.close(); // Закрытие файла
    }

    // Функция для чтения файла
    void loadFromFile(const string& filename) {
        ifstream file(filename); // Открытие файла для чтения
        if (file.is_open()) { // Если файл открыт
            string line;
            while (getline(file, line)) { // Чтение строк из файла
                stringstream ss(line); // Создание строки потока
                string value;
                while (ss >> value) { // Чтение значений из строки
                    if (value == "->" || value == "NULL") continue; // Игнорирование разделителей
                    push(value); // Добавление значения в стек
                }
            }
            file.close(); // Закрытие файла после чтения
        }
    }
};

#endif
