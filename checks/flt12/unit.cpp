// DSP-12: модель, фильтры, открытие из меню.
#define CHECK_MAIN
#define CHECK_QT
#include <QAbstractItemModelTester>
#include <QAction>
#include <QComboBox>
#include <QKeySequence>
#include <QLabel>
#include <QTableView>

#include "check.hpp"
#include "main_window.hpp"

namespace {
struct Widgets {
    QTableView* view;
    QComboBox* kind;
    QComboBox* state;
    QLabel* shown;
    bool ok() const { return view && view->model() && kind && state && shown; }
};
Widgets ui(MainWindow& w) {
    return {w.findChild<QTableView*>("requestsTable"), w.findChild<QComboBox*>("kindFilter"),
            w.findChild<QComboBox*>("stateFilter"), w.findChild<QLabel*>("shownLabel")};
}
bool pick(QComboBox* box, const char* text) {
    const int i = box->findText(QString::fromUtf8(text));
    if (i < 0) return false;
    box->setCurrentIndex(i);
    return true;
}
}  // namespace

TEST("таблица — QTableView с моделью, модель соблюдает правила Qt") {
    MainWindow w;
    const Widgets u = ui(w);
    CHECK(u.ok());
    if (!u.ok()) return;
    QAbstractItemModelTester tester(u.view->model(), QAbstractItemModelTester::FailureReportingMode::Fatal);
    CHECK(w.open_file("checks/rep4/data/month.csv"));
    CHECK_EQ(u.view->model()->rowCount(), 400);
    CHECK_EQ(u.kind->count(), 7);
    CHECK_EQ(u.kind->itemText(0).toStdString(), std::string("все виды"));
    CHECK_EQ(u.state->count(), 7);
    CHECK_EQ(u.state->itemText(1).toStdString(), std::string("открытые"));
    CHECK_EQ(u.shown->text().toStdString(), std::string("показано: 400"));
}

TEST("фильтры по виду и состоянию") {
    MainWindow w;
    const Widgets u = ui(w);
    if (!u.ok()) { CHECK(false); return; }
    w.open_file("checks/rep4/data/month.csv");
    CHECK(pick(u.kind, "лифт"));
    CHECK_EQ(u.view->model()->rowCount(), 35);
    CHECK_EQ(u.shown->text().toStdString(), std::string("показано: 35"));
    CHECK(pick(u.state, "открытые"));
    CHECK_EQ(u.view->model()->rowCount(), 11);
    CHECK(pick(u.kind, "все виды"));
    CHECK_EQ(u.view->model()->rowCount(), 89);
    CHECK(pick(u.state, "закрыта"));
    CHECK_EQ(u.view->model()->rowCount(), 176);
    CHECK(pick(u.kind, "сантехника"));
    CHECK_EQ(u.view->model()->rowCount(), 69);
    for (int r = 0; r < u.view->model()->rowCount(); ++r) {
        CHECK_EQ(u.view->model()->data(u.view->model()->index(r, 3)).toString().toStdString(), std::string("сантехника"));
        if (r > 3) break;
    }
    CHECK(pick(u.state, "все"));
    CHECK(pick(u.kind, "все виды"));
    CHECK_EQ(u.view->model()->rowCount(), 400);
}

TEST("фильтр переживает открытие другого файла") {
    MainWindow w;
    const Widgets u = ui(w);
    if (!u.ok()) { CHECK(false); return; }
    w.open_file("checks/rep4/data/month.csv");
    pick(u.state, "открытые");
    w.open_file("checks/rep4/data/week.csv");
    CHECK_EQ(u.view->model()->rowCount(), 2);
    CHECK_EQ(u.shown->text().toStdString(), std::string("показано: 2"));
}

TEST("меню: открыть файл") {
    MainWindow w;
    const auto* a = w.findChild<QAction*>("openAction");
    CHECK(a != nullptr);
    if (a != nullptr) CHECK(a->shortcut() == QKeySequence("Ctrl+O"));
}
