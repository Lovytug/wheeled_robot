# Базовый C++ Code Style

## 1. Форматирование

Используй **4 пробела**, без табов.

Фигурная скобка всегда находится **на отдельной строке**:

```cpp
if (condition)
{
    DoSomething();
}
else
{
    DoSomethingElse();
}
```

Для функций:

```cpp
void PrintMessage(const std::string& message)
{
    std::cout << message << '\n';
}
```

Для классов:

```cpp
class User
{
public:
    void Print();

private:
    int _id;
};
```

---

## 2. Имена

### Переменные и функции — `camelCase`

```cpp
int userCount = 10;
std::string userName;

void calculateAverage();
bool isValid();
```

Не используй `snake_case`:

```cpp
// Плохо
int user_count;
bool is_valid();
```

### Классы и структуры — `PascalCase`

```cpp
class UserAccount
{
};

struct PlayerStats
{
    int health;
    int score;
};
```

### Константы — `UPPER_SNAKE_CASE`

```cpp
constexpr int MAX_PLAYERS = 100;
constexpr double PI = 3.141592653589793;
```

Не используй префикс `k`:

```cpp
// Плохо
constexpr int kMaxPlayers = 100;
constexpr double kPi = 3.141592653589793;
```

---

## 3. Приватные переменные

Для private-полей используй `_` **перед** именем:

```cpp
class User
{
public:
    explicit User(int id)
        : _id(id)
    {
    }

    int id() const
    {
        return _id;
    }

private:
    int _id;
    std::string _name;
};
```

То есть:

```cpp
int _id;
std::string _userName;
bool _isActive;
```

Не:

```cpp
int id_;
std::string userName_;
bool isActive_;
```

---

## 4. `const`

Если значение не должно изменяться — используй `const`:

```cpp
const int maxPlayers = 10;
const std::string playerName = "Alex";
```

Параметры:

```cpp
void PrintUser(const std::string& userName)
{
    std::cout << userName << '\n';
}
```

Методы, которые не изменяют объект:

```cpp
class User
{
public:
    const std::string& name() const
    {
        return _name;
    }

private:
    std::string _name;
};
```

---

## 5. Параметры функций

Маленькие типы передавай по значению:

```cpp
void SetAge(int age);
void SetScore(double score);
```

Большие read-only объекты — через `const&`:

```cpp
void PrintName(const std::string& name);

void ProcessUsers(const std::vector<User>& users);
```

Если функция должна изменить объект:

```cpp
void UpdateUser(User& user);
```

---

## 6. Указатели и ссылки

Если объект должен существовать — используй ссылку:

```cpp
void ProcessUser(const User& user);
```

Если `nullptr` является допустимым состоянием — указатель:

```cpp
User* FindUser(int id);
```

Используй `nullptr`:

```cpp
User* user = nullptr;
```

Не используй `NULL`:

```cpp
// Плохо
User* user = NULL;
```

---

## 7. Умные указатели

Не используй ручное управление памятью:

```cpp
// Плохо
User* user = new User();
delete user;
```

Предпочитай RAII:

```cpp
auto user = std::make_unique<User>();
```

Если действительно требуется shared ownership:

```cpp
auto user = std::make_shared<User>();
```

---

## 8. `auto`

Используй `auto`, когда тип очевиден из контекста:

```cpp
auto user = GetUser();
auto userCount = users.size();
```

В range-based loop:

```cpp
for (const auto& user : users)
{
    std::cout << user.name() << '\n';
}
```

Если тип важен для понимания кода, можно написать его явно:

```cpp
double averageScore = CalculateAverage();
```

---

## 9. `enum class`

Используй `enum class`:

```cpp
enum class WeaponType
{
    Sword,
    Bow,
    Magic
};

WeaponType weapon = WeaponType::Sword;
```

---

## 10. Условия

```cpp
if (user.isActive())
{
    ActivateUser(user);
}
```

Для `else`:

```cpp
if (user.isActive())
{
    ActivateUser(user);
}
else
{
    DeactivateUser(user);
}
```

Для сложных условий лучше использовать промежуточную переменную:

```cpp
const bool isValid = user.isActive() && user.age() >= 18;

if (isValid)
{
    ProcessUser(user);
}
```

---

## 11. Ранний `return`

Предпочитай ранний выход вместо глубокой вложенности:

```cpp
void ProcessUser(const User& user)
{
    if (!user.isValid())
    {
        return;
    }

    if (!user.isActive())
    {
        return;
    }

    Process(user);
}
```

Вместо:

```cpp
void ProcessUser(const User& user)
{
    if (user.isValid())
    {
        if (user.isActive())
        {
            Process(user);
        }
    }
}
```

---

## 12. `switch`

```cpp
switch (weapon)
{
    case WeaponType::Sword:
        AttackWithSword();
        break;

    case WeaponType::Bow:
        AttackWithBow();
        break;

    case WeaponType::Magic:
        CastSpell();
        break;
}
```

---

## 13. Константы

Не используй магические числа:

```cpp
// Плохо
if (health > 100)
{
    // ...
}
```

Лучше:

```cpp
constexpr int MAX_HEALTH = 100;

if (health > MAX_HEALTH)
{
    // ...
}
```

Для глобальных и compile-time констант также используй `UPPER_SNAKE_CASE`:

```cpp
constexpr int MAX_PLAYERS = 100;
constexpr int DEFAULT_HEALTH = 100;
constexpr double PI = 3.141592653589793;
```

Если константа является частью класса:

```cpp
class Player
{
public:
    static constexpr int MAX_HEALTH = 100;
    static constexpr int MAX_MANA = 50;
};
```

---

## 14. Комментарии

Комментарии должны объяснять **почему**, а не очевидное **что**.

Плохо:

```cpp
// Increment counter
counter++;
```

Лучше:

```cpp
// The server uses a 1-based player index.
++playerIndex;
```

---

## 15. Заголовочные файлы

### `User.h`

```cpp
#pragma once

#include <string>

class User
{
public:
    explicit User(int id);

    int id() const;
    const std::string& name() const;

private:
    int _id;
    std::string _name;
};
```

### `User.cpp`

```cpp
#include "User.h"

User::User(int id)
    : _id(id)
{
}

int User::id() const
{
    return _id;
}

const std::string& User::name() const
{
    return _name;
}
```

---

## 16. Структура проекта

```text
MyProject/
├── CMakeLists.txt
├── include/
│   └── User.h
├── src/
│   ├── main.cpp
│   └── User.cpp
└── tests/
    └── UserTest.cpp
```

Имена файлов — `PascalCase`, если они соответствуют классу:

```text
User.h
User.cpp
Player.h
Player.cpp
GameManager.h
GameManager.cpp
```

---

## 17. Полный пример

```cpp
#include <iostream>
#include <string>
#include <vector>

constexpr int MIN_SCORE = 200;

class Player
{
public:
    Player(std::string name, int score)
        : _name(std::move(name)),
          _score(score)
    {
    }

    const std::string& name() const
    {
        return _name;
    }

    int score() const
    {
        return _score;
    }

private:
    std::string _name;
    int _score;
};

int main()
{
    const std::vector<Player> players =
    {
        {"Alice", 100},
        {"Bob", 250},
        {"Charlie", 175}
    };

    for (const auto& player : players)
    {
        if (player.score() < MIN_SCORE)
        {
            continue;
        }

        std::cout
            << player.name()
            << ": "
            << player.score()
            << '\n';
    }

    return 0;
}
```

## Короткий чеклист

```text
✓ 4 пробела
✓ camelCase для переменных
✓ camelCase для функций
✓ PascalCase для классов и структур
✓ UPPER_SNAKE_CASE для констант
✓ _camelCase для private-полей
✓ Фигурные скобки на отдельной строке
✓ const по умолчанию
✓ nullptr вместо NULL
✓ enum class вместо enum
✓ RAII вместо ручного new/delete
✓ unique_ptr предпочтительнее shared_ptr
✓ const T& для больших read-only объектов
✓ Ранний return вместо глубокой вложенности
✓ constexpr для compile-time констант
✓ Комментарии объясняют "почему"
✓ clang-format для автоматического форматирования
✓ clang-tidy для поиска проблем
```
