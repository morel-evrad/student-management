
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
using namespace std;

struct Student {
    int id = 0;
    string name;
    vector<double> grades; 
};
double calculateAverage(const Student& student) {
    if (student.grades.empty()) {
        return 0.0;
    }

    double sum = 0.0;

    for (double grade : student.grades) {
        sum += grade;
    }

    return sum / student.grades.size();
}
void saveStudents(const vector<Student>& students) {
    ofstream file("students.txt");

    if (!file.is_open()) {
        cout << "Ошибка сохранения файла!\n";
        return;
    }

    for (const auto& student : students) {
        file << student.id << "\n";
        file << student.name << "\n";
        file << student.grades.size() << "\n";

        for (double grade : student.grades) {
            file << grade << "\n";
        }
    }

    file.close();
    cout << "Данные успешно сохранены!\n";
}

void loadStudents(vector<Student>& students) {
    ifstream file("students.txt");

    if (!file.is_open()) {
        return;
    }

    Student s;

    while (file >> s.id) {
        file.ignore(10000,'\n');

        if (!getline(file, s.name)) {
            break;
        }
        size_t gradeCount = 0;

        if (!(file >> gradeCount)) {
            break;
        }

        s.grades.clear();

        for (size_t i = 0; i < gradeCount; i++) {
            double grade;

            if (!(file >> grade)) {
                return;
            }

            s.grades.push_back(grade);
        }
        students.push_back(s);
    }

    file.close();
}

template <typename Number>
bool readNumber(Number& number) {
    string line;
    if (!getline(cin, line)) {
        return false;
    }

    istringstream input(line);
    return (input >> number) && (input >> ws).eof();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    vector<Student> students;
    loadStudents(students);

    while (true) {
        cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===\n";
        cout << "1. Добавить студента\n";
        cout << "2. Показать список студентов\n";
        cout << "3. Найти студента\n";
        cout << "4. Удалить студента\n";
        cout << "5. Изменить данные студента\n";
        cout << "6. Добавить оценку студенту\n";
        cout << "0. Выход\n";
        cout << "Выберите действие: ";

        int choix;
        if (!readNumber(choix)) {
            if (!cin) {
                break;
            }
            cout << "Ошибка ввода!\n";
            continue;
        }
        if (choix == 0) {
            break;
        }

        if (choix == 1) {
            Student s;
            cout << "Введите ID студента: ";
            if (!readNumber(s.id)) {
                cout << "Ошибка ввода ID!\n";
                continue;
            }

            cout << "Введите имя студента: ";
            getline(cin, s.name);

            students.push_back(s);
            saveStudents(students);
            cout << "Студент успешно добавлен!\n";
        }
        else if (choix == 2) {
            if (students.empty()) {
                cout << "Список студентов пуст.\n";
            }
            else {
                for (const auto& s : students) {
                    cout << "ID: " << s.id
                        << " | Имя: " << s.name << "\n";
                }
            }
        }
        else if (choix == 3) {
            int id;
            bool found = false;

            cout << "Введите ID для поиска: ";
            if (!readNumber(id)) {
                cout << "Ошибка ввода ID!\n";
                continue;
            }

            for (const auto& s : students) {
                if (s.id == id) {
                    cout << "Найден студент: " << s.name << "\n";
                    found = true;
                    break;
                }
            }

            if (!found) {
                cout << "Студент не найден.\n";
            }
        }
        else if (choix == 4) {
            int id;
            cout << "Введите ID студента для удаления: ";
            if (!readNumber(id)) {
                cout << "Ошибка ввода ID!\n";
                continue;
            }

            bool found = false;

            for (auto it = students.begin(); it != students.end(); ++it) {
                if (it->id == id) {
                    students.erase(it);
                    saveStudents(students);

                    cout << "Студент успешно удалён!\n";
                    found = true;
                    break;
                }
            }

            if (!found) {
                cout << "Студент не найден!\n";
            }
        }
        else if (choix == 5) {
            int id;
            cout << "Введите ID студента: ";
            if (!readNumber(id)) {
                cout << "Ошибка ввода ID!\n";
                continue;
            }

            bool found = false;

            for (auto& student : students) {
                if (student.id == id) {
                    found = true;

                    cout << "Текущее имя: " << student.name << "\n";
                    cout << "Введите новое имя: ";

                    getline(cin, student.name);

                    saveStudents(students);

                    cout << "Данные успешно изменены!\n";
                    break;
                }
            }

            if (!found) {
                cout << "Студент не найден!\n";
            }
        }
        else if (choix == 6) {
            int id;
            cout << "Введите ID студента: ";

            if (!readNumber(id)) {
                cout << "Ошибка ввода ID!\n";
                continue;
            }

            bool found = false;

            for (auto& student : students) {
                if (student.id == id) {
                    found = true;

                    double grade;
                    cout << "Введите оценку (0-20): ";

                    if (!readNumber(grade) || grade < 0 || grade > 20) {
                        cout << "Ошибка! Оценка должна быть от 0 до 20.\n";
                        break;
                    }

                    student.grades.push_back(grade);
                    saveStudents(students);

                    cout << "Оценка успешно добавлена!\n";
                    cout << "Средний балл: "
                        << calculateAverage(student) << "\n";

                    break;
                }
            }

            if (!found) {
                cout << "Студент не найден!\n";
            }
        }
        else {
            cout << "Неверный выбор!\n";
        }

    }

    cout << "Завершение программы.\n";
    return 0;
}
