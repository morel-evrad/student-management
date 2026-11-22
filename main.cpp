
#include <iostream>
using namespace std;

int main() {
    int choix;

    do {
        cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===\n";
        cout << "1. Добавить студента\n";
        cout << "2. Показать список студентов\n";
        cout << "3. Найти студента\n";
        cout << "4. Управление оценками\n";
        cout << "0. Выход\n";
        cout << "Выберите действие: ";

        if (!(cin >> choix)) {
            cout << "Ошибка ввода!\n";
            return 1;
        }

        switch (choix) {
            case 1:
            case 2:
            case 3:
            case 4:
                cout << "Функция будет добавлена позже.\n";
                break;
            case 0:
                cout << "Завершение программы.\n";
                break;
            default:
                cout << "Неверный выбор!\n";
        }
    } while (choix != 0);

    return 0;
}
