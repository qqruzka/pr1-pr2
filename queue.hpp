#ifndef HEADER_QUEUE_HPP
#define HEADER_QUEUE_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;
// Узел для очереди
struct NodeQ {
    string value; // Значение узла
    NodeQ* next; // Указатель на следующий узел
};

// Структура очереди
class Queue {
private:
    NodeQ* front; // Указатель на передний элемент
    NodeQ* rear; // Указатель на задний элемент
public:
    // Конструктор очереди
    Queue() {
        front = nullptr; // Инициализация указателей
        rear = nullptr;
    }

    // Функция для добавления элемента в очередь
    void push(const string& value) {
        NodeQ* newNodeQ = new NodeQ(); // Создание нового узла
        newNodeQ->value = value; // Запись значения
        newNodeQ->next = nullptr; // Установка указателя на следующий узел
        if (rear == nullptr) { // Если очередь пуста
            front = rear = newNodeQ; // Установка указателей
        } else {
            rear->next = newNodeQ; // Присоединение нового узла к концу
            rear = newNodeQ; // Обновление указателя на задний элемент
        }
        writeToFile(); // Сохранение состояния очереди в файл
    }
    // Функция для удаления элемента из очереди
    void pop() {
        if (front == nullptr) { // Если очередь пуста
            cout << "Queue is empty, nothing to remove." << endl; // Сообщение об ошибке
            return;
        }
        NodeQ* temp = front; // Сохранение переднего узла
        front = front->next; // Перемещение указателя на следующий узел
        if (front == nullptr) { // Если очередь стала пустой
            rear = nullptr; // Обнуление указателя на задний элемент
        }
        delete temp; // Освобождение памяти
        writeToFile(); // Сохранение состояния очереди в файл
    }
    // Функция для чтения элементов очереди
    void read() {
        if (front == nullptr) { // Если очередь пуста
            cout << "Queue is empty." << endl; // Сообщение об ошибке
            return;
        }
        NodeQ* temp = front; // Указатель на передний элемент
        while (temp) {
            cout << temp->value; // Вывод значения
            if (temp->next) cout << " <- "; // Разделитель
            temp = temp->next; // Переход к следующему узлу
        }
        cout << " <- NULL" << endl; // Завершение вывода
    }
    // Функция для записи состояния очереди в файл
    void writeToFile() {
        ofstream file("queue.txt"); // Открытие файла для записи
        if (!file.is_open()) {
            cout << "Unable to open file for writing." << endl; // Сообщение об ошибке
            return;
        }
        NodeQ* temp = front; // Указатель на передний элемент
        while (temp) {
            file << temp->value; // Запись значения
            if (temp->next) {
                file << " <- "; // Разделитель
            }
            temp = temp->next; // Переход к следующему узлу
        }
        file << " <- NULL" << endl; // Завершение записи
        file.close(); // Закрытие файла
    }
    // чтение файла
    void loadFromFile(const string& filename) {
        ifstream file(filename); // Открытие файла для чтения
        if (!file.is_open()) {
            return;
        }
        string value; // Переменная для хранения значения
        NodeQ* previous = nullptr; // Указатель на предыдущий узел
        while (file >> value) { // Чтение значений из файла
            if (value == "NULL") break; // Завершение при встрече NULL

            if (value == "<-") continue; // Игнорирование разделителя

            NodeQ* newNode = new NodeQ(); // Создание нового узла
            newNode->value = value; // Запись значения
            newNode->next = nullptr; // Установка указателя на следующий узел

            if (previous == nullptr) { // Если это первый узел
                front = newNode; // Установка указателя на передний элемент
            } else {
                previous->next = newNode; // Присоединение нового узла к предыдущему
            }

            previous = newNode; // Обновление указателя на предыдущий узел
            rear = newNode; // Обновление указателя на задний элемент
        }

        file.close(); // Закрытие файла
    }
};
#endif
