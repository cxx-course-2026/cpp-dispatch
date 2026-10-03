# DSP-10. Окно диспетчера

**Автор:** Марина Кольцова, тимлид · **Неделя:** 10

К 10 декабря заказчик ждёт версию с окнами [Б8]. Командная строка остаётся — для скриптов и для нас, — а диспетчеру нужно окно: открыл файл и видит все заявки, аварийные — сверху и подсвечены [Б9]. Вся логика уже есть в `dispatch_core`; окно — второй интерфейс к ней, и ни одного правила заказчика в нём быть не должно.

## Что сделать

### 1. Сборка

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)

add_library(dispatch_gui_lib STATIC src/gui/main_window.cpp)          # всё окно, кроме main
target_include_directories(dispatch_gui_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)
target_link_libraries(dispatch_gui_lib PUBLIC dispatch_core Qt6::Widgets)

add_executable(dispatch-gui src/gui/main.cpp)
target_link_libraries(dispatch-gui PRIVATE dispatch_gui_lib)
```

Окно — в библиотеке, чтобы его могли собрать и проверить тесты; программа `dispatch-gui` — только `main`. Файл заявок — первый аргумент: `./build/dispatch-gui requests.csv`.

### 2. `src/gui/main_window.hpp`

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool open_file(const QString& path);   // false — файл не прочитан
};
```

- Центральный виджет — таблица с именем объекта `requestsTable`: `QTableWidget` (или `QTableView` со своей моделью — в DSP-12 мы к этому придём; проверки смотрят через общий интерфейс `QAbstractItemView` и его модель), семь столбцов: `№`, `Создана`, `Срочность`, `Вид работ`, `Состояние`, `Адрес`, `Мастер`. Адрес — `Лесная 12, кв. 45`, пустой мастер — `—`.
- Порядок строк: сначала открытые заявки (новая, назначена) в порядке `open_queue` из DSP-8, потом остальные по возрастанию номера.
- Строки открытых **аварийных** заявок — с фоном `QColor(255, 220, 220)` во всех ячейках.
- Таблица только для чтения, выделяется строка целиком.
- В строке состояния — постоянная надпись `QLabel` с именем `summaryLabel`: `заявок: 400, открытых: 89, ошибок в файле: 0` (ошибки — строки файла, не прошедшие проверку).
- Заголовок окна — `Диспетчер — month.csv` (только имя файла).
- Файл не прочитан (`FileError`) — таблица пустая, в `summaryLabel` — текст ошибки, `open_file` возвращает `false`. Никаких модальных окон: они остановили бы тесты.

Для перевода строк: `QString::fromStdString(s)` и `s.toStdString()` — в обе стороны UTF-8.

## Критерии готовности

- `make check-gui10` и `make check-sum10` зелёные, проверки прошлых тикетов — тоже.
- `./build/dispatch-gui checks/rep4/data/month.csv` открывает окно; в PR — скриншот.
- В описании PR: какие классы `dispatch_core` использует окно и есть ли в `main_window.cpp` хоть одно правило заказчика? Если есть — вынесите его в библиотеку.
