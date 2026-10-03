# DSP-5. Библиотека `dispatch_core` и первые тесты

**Автор:** Марина Кольцова, тимлид · **Неделя:** 5

С недели 10 у «Диспетчера» появится второе лицо — окно на Qt. Разбор заявок, поиск, отчёт у окна и у командной строки общие, и копировать их нельзя. Поэтому сейчас, пока кода немного, разделяем: **библиотека** `dispatch_core` — вся логика, **программа** `dispatch` — только разбор аргументов и печать. И начинаем писать тесты: до сих пор программу проверяли только проверки курса, а к релизу 0.1 на следующей неделе у команды должны быть свои.

## Что сделать

### 1. Раскладка

```text
include/dispatch/request.hpp   Request, ParseError, parse_request
include/dispatch/store.hpp     FileError, read_requests, load_requests, find_request
include/dispatch/text.hpp      utf8_length, utf8_lower, pad_left, pad_right
src/core/*.cpp                 их реализация — библиотека dispatch_core
src/cli/main.cpp               программа dispatch
tests/test_*.cpp               ваши тесты — программа dispatch_tests
```

В `CMakeLists.txt`:

```cmake
add_library(dispatch_core STATIC src/core/request.cpp src/core/store.cpp src/core/text.cpp)
target_include_directories(dispatch_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(dispatch src/cli/main.cpp)
target_link_libraries(dispatch PRIVATE dispatch_core)
```

Всё публичное — в пространстве имён `dispatch`. Вспомогательные функции реализации — в безымянном пространстве имён внутри `.cpp`, в заголовки они не попадают. Заголовки подключаются как `#include "dispatch/request.hpp"`.

### 2. Интерфейс библиотеки

Проверки DSP-5 — это модульные тесты курса (`checks/lib5/unit.cpp`), которые линкуются с **вашей** библиотекой. Поэтому имена и сигнатуры — ровно такие:

```cpp
// dispatch/request.hpp
namespace dispatch {
struct Request {
    int id = 0;
    std::string created, house;
    int entrance = 0, flat = 0;
    std::string phone, kind, priority, state, master, done, text;
};
class ParseError : public std::runtime_error { using std::runtime_error::runtime_error; };
Request parse_request(const std::string& line);   // строка без '\r'; тексты ошибок — из DSP-3
}

// dispatch/store.hpp
namespace dispatch {
class FileError : public std::runtime_error { using std::runtime_error::runtime_error; };
// Заявки из потока: заголовок (с BOM или без), строки с '\r' или без. Ошибки строк — в errors
// («строка N: …»), неверный заголовок — FileError.
std::vector<Request> read_requests(std::istream& in, std::ostream& errors);
std::vector<Request> load_requests(const std::string& path, std::ostream& errors);  // + «не открыть файл»
const Request* find_request(const std::vector<Request>& all, int id);
}

// dispatch/text.hpp
namespace dispatch {
std::size_t utf8_length(std::string_view s);
std::string utf8_lower(std::string_view s);
std::string pad_right(std::string_view s, std::size_t width);
std::string pad_left(std::string_view s, std::size_t width);
}
```

`read_requests` из потока — новая функция: `load_requests` открывает файл и зовёт её. Так библиотеку можно тестировать без файлов: `std::istringstream in("номер;…\n17;…\n");`.

### 3. Свои тесты

- Программа `dispatch_tests` из `tests/test_*.cpp`, линкуется с `dispatch_core`. Для тестов возьмите маленький фреймворк курса `checks/check.hpp` (как в лабораторных): `target_include_directories(dispatch_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/checks)`, в одном из файлов — `#define CHECK_MAIN` перед `#include "check.hpp"`.
- Не меньше **8** тестов (`TEST("…")`), и среди них: разбор правильной строки; хотя бы три разных ошибки разбора с проверкой, что брошен `ParseError`; `read_requests` с BOM и `\r\n`; пустой результат поиска; дополнение русской строки до ширины.
- `enable_testing()` и `add_test(NAME dispatch_tests COMMAND dispatch_tests)` — тогда тесты запускаются и командой `ctest --test-dir build`.

Поведение `dispatch` не меняется: проверки прошлых тикетов должны остаться зелёными.

## Критерии готовности

- `make check-lib5` и `make check-tst5` зелёные, проверки DSP-1…DSP-4 — тоже.
- PR `dsp-5` в `main`. В описании ответьте: почему `Request` и `ParseError` можно определять в заголовке, а тело `parse_request` — нельзя? Что изменилось бы, если бы `dispatch_core` была `SHARED`?
