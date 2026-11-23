
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <unordered_set>
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <string_view>
#include <limits>
#include <fstream>
#include <iomanip>
#include <locale>
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
void printStudent(const Student& student) {
    cout << "ID: " << student.id << " | Имя: " << student.name;
    if (student.grades.empty()) {
        cout << " | Нет оценок\n";
        return;
    }
    cout << " | Оценки:";
    for (double grade : student.grades) {
        cout << ' ' << grade;
    }
    cout << " | Средний балл: " << calculateAverage(student) << '\n';
}

bool saveStudents(const vector<Student>& students, const filesystem::path& dataPath) {
    wchar_t temporaryFile[MAX_PATH];
    const auto directory = dataPath.parent_path().empty() ? filesystem::path(L".") : dataPath.parent_path();
    if (!GetTempFileNameW(directory.c_str(), L"stu", 0, temporaryFile)) {
        cerr << "Ошибка сохранения файла!\n";
        return false;
    }

    ofstream file{filesystem::path(temporaryFile)};

    file.imbue(locale::classic());
    file << setprecision(numeric_limits<double>::max_digits10);
    for (const auto& student : students) {
        file << student.id << "\n";
        file << student.name << "\n";
        file << student.grades.size() << "\n";

        for (double grade : student.grades) {
            file << grade << "\n";
        }
    }

    file.close();
    if (!file || !MoveFileExW(temporaryFile, dataPath.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        error_code ignored;
        filesystem::remove(temporaryFile, ignored);
        cerr << "Ошибка сохранения файла!\n";
        return false;
    }
    cout << "Данные успешно сохранены!\n";
    return true;
}

template <typename Number>
bool parseNumber(const string& line, Number& number) {
    istringstream input(line);
    input.imbue(locale::classic());
    return (input >> number) && (input >> ws).eof();
}

bool validGrade(double grade) {
    return isfinite(grade) && grade >= 0 && grade <= 20;
}

bool loadStudents(vector<Student>& students, const filesystem::path& dataPath) {
    error_code error;
    if (!filesystem::exists(dataPath, error)) {
        return !error;
    }

    ifstream file(dataPath);
    if (!file) {
        return false;
    }

    vector<Student> loaded;
    unordered_set<int> ids;
    string line;
    while (getline(file, line)) {
        Student student;
        long long gradeCount;
        if (!parseNumber(line, student.id) || !ids.insert(student.id).second ||
            !getline(file, student.name) || !getline(file, line) ||
            !parseNumber(line, gradeCount) || gradeCount < 0) {
            return false;
        }
        for (long long i = 0; i < gradeCount; ++i) {
            double grade;
            if (!getline(file, line) || !parseNumber(line, grade) || !validGrade(grade)) {
                return false;
            }
            student.grades.push_back(grade);
        }
        loaded.push_back(move(student));
    }
    if (!file.eof() || file.bad()) {
        return false;
    }
    students.swap(loaded);
    return true;
}

template <typename Number>
bool readNumber(Number& number) {
    string line;
    return getline(cin, line) && parseNumber(line, number);
}

bool readName(string& name) {
    string line;
    if (!getline(cin, line)) {
        return false;
    }
    const auto first = line.find_first_not_of(" \t\r\n\v\f");
    if (first == string::npos) {
        cout << "Имя не может быть пустым.\n";
        return false;
    }
    const auto last = line.find_last_not_of(" \t\r\n\v\f");
    name = line.substr(first, last - first + 1);
    return true;
}

vector<Student>::iterator readStudent(vector<Student>& students) {
    int id;
    cout << "Введите ID студента: ";
    if (!readNumber(id)) {
        cout << "Ошибка ввода ID!\n";
        return students.end();
    }
    const auto student = find_if(students.begin(), students.end(), [id](const Student& entry) {
        return entry.id == id;
    });
    if (student == students.end()) {
        cout << "Студент не найден!\n";
    }
    return student;
}

vector<double>::iterator readGradePosition(Student& student) {
    auto& grades = student.grades;
    if (grades.empty()) {
        cout << "Нет оценок.\n";
        return grades.end();
    }
    for (size_t i = 0; i < grades.size(); ++i) {
        cout << i + 1 << ". " << grades[i] << '\n';
    }
    cout << "Введите номер оценки: ";
    vector<double>::difference_type position;
    if (!readNumber(position) || position < 1 || static_cast<size_t>(position) > grades.size()) {
        cout << "Неверный номер оценки.\n";
        return grades.end();
    }
    return grades.begin() + (position - 1);
}

void printGradeSummary(const vector<Student>& students) {
    size_t gradedStudents = 0;
    size_t gradeCount = 0;
    double sum = 0;
    double lowest = 20;
    double highest = 0;
    for (const auto& student : students) {
        if (!student.grades.empty()) {
            ++gradedStudents;
        }
        for (double grade : student.grades) {
            ++gradeCount;
            sum += grade;
            if (grade < lowest) {
                lowest = grade;
            }
            if (grade > highest) {
                highest = grade;
            }
        }
    }
    cout << "Студентов с оценками: " << gradedStudents << " / " << students.size() << '\n';
    cout << "Всего оценок: " << gradeCount << '\n';
    if (gradeCount != 0) {
        cout << "Среднее всех оценок: " << sum / gradeCount << '\n'
            << "Минимум: " << lowest << " | Максимум: " << highest << '\n';
    }
}

class DataFileLock {
public:
    explicit DataFileLock(const filesystem::path& path)
        : handle(CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)) {}

    ~DataFileLock() {
        if (isHeld()) {
            CloseHandle(handle);
        }
    }

    DataFileLock(const DataFileLock&) = delete;
    DataFileLock& operator=(const DataFileLock&) = delete;

    bool isHeld() const { return handle != INVALID_HANDLE_VALUE; }

private:
    HANDLE handle;
};

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    filesystem::path dataPath = L"students.txt";
    if (argc != 1) {
        if (argc != 3 || wstring_view(argv[1]) != L"--data" || !*argv[2]) {
            cerr << "Использование: StudentManagement.exe [--data <файл>]\n";
            return 2;
        }
        dataPath = argv[2];
    }

    auto lockPath = dataPath;
    lockPath += L".lock";
    const DataFileLock lock(lockPath);
    if (!lock.isHeld()) {
        cerr << "Не удалось заблокировать файл данных. Закройте другую копию программы и проверьте доступ к папке.\n";
        return 1;
    }

    vector<Student> students;
    if (!loadStudents(students, dataPath)) {
        cerr << "Не удалось прочитать " << reinterpret_cast<const char*>(dataPath.u8string().c_str()) << ". Проверьте файл данных.\n";
        return 1;
    }

    while (true) {
        cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===\n";
        cout << "1. Добавить студента\n";
        cout << "2. Показать список студентов\n";
        cout << "3. Найти студента\n";
        cout << "4. Удалить студента\n";
        cout << "5. Изменить данные студента\n";
        cout << "6. Добавить оценку студенту\n";
        cout << "7. Найти студентов по имени\n";
        cout << "8. Исправить оценку\n";
        cout << "9. Удалить оценку\n";
        cout << "10. Показать статистику оценок\n";
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

            if (s.id <= 0) {
                cout << "ID должен быть положительным.\n";
                continue;
            }
            if (any_of(students.begin(), students.end(), [&](const Student& student) {
                return student.id == s.id;
            })) {
                cout << "Студент с таким ID уже существует.\n";
                continue;
            }

            cout << "Введите имя студента: ";
            if (!readName(s.name)) {
                continue;
            }

            students.push_back(s);
            if (!saveStudents(students, dataPath)) {
                return 1;
            }
            cout << "Студент успешно добавлен!\n";
        }
        else if (choix == 2) {
            cout << "Всего студентов: " << students.size() << '\n';
            if (students.empty()) {
                cout << "Список студентов пуст.\n";
            }
            else {
                for (const auto& s : students) {
                    printStudent(s);
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
                    printStudent(s);
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
                    found = true;
                    cout << "Удалить " << it->name << "? (y/N): ";
                    string answer;
                    if (!getline(cin, answer) || (answer != "y" && answer != "Y")) {
                        cout << "Удаление отменено.\n";
                        break;
                    }
                    students.erase(it);
                    if (!saveStudents(students, dataPath)) {
                        return 1;
                    }

                    cout << "Студент успешно удалён!\n";
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

                    if (!readName(student.name)) {
                        break;
                    }

                    if (!saveStudents(students, dataPath)) {

                        return 1;

                    }

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

                    if (!readNumber(grade) || !validGrade(grade)) {
                        cout << "Ошибка! Оценка должна быть от 0 до 20.\n";
                        break;
                    }

                    student.grades.push_back(grade);
                    if (!saveStudents(students, dataPath)) {
                        return 1;
                    }

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
        else if (choix == 7) {
            cout << "Введите имя или его часть: ";
            string query;
            if (!readName(query)) {
                continue;
            }
            bool found = false;
            for (const auto& student : students) {
                if (student.name.find(query) != string::npos) {
                    printStudent(student);
                    found = true;
                }
            }
            if (!found) {
                cout << "Студент не найден.\n";
            }
        }
        else if (choix == 8) {
            const auto student = readStudent(students);
            if (student == students.end()) {
                continue;
            }
            const auto selected = readGradePosition(*student);
            if (selected == student->grades.end()) {
                continue;
            }
            double grade;
            cout << "Введите новую оценку (0-20): ";
            if (!readNumber(grade) || !validGrade(grade)) {
                cout << "Ошибка! Оценка должна быть от 0 до 20.\n";
                continue;
            }
            *selected = grade;
            if (!saveStudents(students, dataPath)) {
                return 1;
            }
            cout << "Оценка изменена.\n";
        }
        else if (choix == 9) {
            const auto student = readStudent(students);
            if (student == students.end()) {
                continue;
            }
            const auto selected = readGradePosition(*student);
            if (selected == student->grades.end()) {
                continue;
            }
            cout << "Удалить оценку " << *selected << "? (y/N): ";
            string answer;
            if (!getline(cin, answer) || (answer != "y" && answer != "Y")) {
                cout << "Удаление отменено.\n";
                continue;
            }
            student->grades.erase(selected);
            if (!saveStudents(students, dataPath)) {
                return 1;
            }
            cout << "Оценка удалена.\n";
        }
        else if (choix == 10) {
            printGradeSummary(students);
        }
        else {
            cout << "Неверный выбор!\n";
        }

    }

    cout << "Завершение программы.\n";
    return 0;
}
