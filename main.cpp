
#include <iostream>
#include <vector>
#include <string>
#include <limits>
using namespace std;

struct Student {
    int id;
    string name;
};

int main() {
    vector<Student> students;
    int choix;

    do {
        cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===\n";
        cout << "1. Добавить студента\n";
        cout << "2. Показать список студентов\n";
        cout << "3. Найти студента\n";
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

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Введите имя студента: ";
            getline(cin, s.name);

            students.push_back(s);
            cout << "Студент успешно добавлен!\n";
        }
        else if (choix == 2) {
            if (students.empty()) {
                cout << "Список студентов пуст.\n";
            } else {
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
        else if (choix != 0) {
            cout << "Неверный выбор!\n";
        }

    } while (choix != 0);

    cout << "Завершение программы.\n";
    return 0;
}
