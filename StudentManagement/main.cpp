
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
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

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    vector<Student> students;
    loadStudents(students);

    int choix;

    do {
        cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===\n";
        cout << "1. Добавить студента\n";
        cout << "2. Показать список студентов\n";
        cout << "3. Найти студента\n";
        cout << "4. Удалить студента\n";
        cout << "5. Изменить данные студента\n";
        cout << "6. Добавить оценку студенту\n";
        cout << "0. Выход\n";
        cout << "Выберите действие: ";

        if (!(cin >> choix)) {
            cout << "Ошибка ввода!\n";
            return 1;
        }

        if (choix == 1) {
            Student s;
            cout << "Введите ID студента: ";
            if (!(cin >> s.id)) {
                cout << "Ошибка ввода ID!\n";
                return 1;
            }

            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
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
            if (!(cin >> id)) {
                cout << "Ошибка ввода ID!\n";
                return 1;
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
            cin >> id;

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
            cin >> id;

            bool found = false;

            for (auto& student : students) {
                if (student.id == id) {
                    found = true;

                    cout << "Текущее имя: " << student.name << "\n";
                    cout << "Введите новое имя: ";

                    cin.ignore(10000, '\n');
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

            if (!(cin >> id)) {
                cout << "Ошибка ввода ID!\n";
                return 1;
            }

            bool found = false;

            for (auto& student : students) {
                if (student.id == id) {
                    found = true;

                    double grade;
                    cout << "Введите оценку (0-20): ";

                    if (!(cin >> grade) || grade < 0 || grade > 20) {
                        cout << "Ошибка! Оценка должна быть от 0 до 20.\n";
                        cin.clear();
                        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
                        break;
                    }

                    student.grades.push_back(grade);

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
        else if (choix != 0) {
            cout << "Неверный выбор!\n";
        }

    } while (choix != 0);

    cout << "Завершение программы.\n";
    return 0;
}
