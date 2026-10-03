// DSP-10: строка состояния, заголовок, ошибки файла.
#define CHECK_MAIN
#define CHECK_QT
#include <QLabel>
#include <QAbstractItemModel>
#include <QAbstractItemView>

#include "check.hpp"
#include "main_window.hpp"

namespace {
std::string summary(MainWindow& w) {
    const QLabel* l = w.findChild<QLabel*>("summaryLabel");
    return l ? l->text().toStdString() : "<нет summaryLabel>";
}
}  // namespace

TEST("итог и заголовок") {
    MainWindow w;
    CHECK(w.open_file("checks/rep4/data/month.csv"));
    CHECK_EQ(summary(w), std::string("заявок: 400, открытых: 89, ошибок в файле: 0"));
    CHECK_EQ(w.windowTitle().toStdString(), std::string("Диспетчер — month.csv"));
}

TEST("ошибки строк считаются, файл из Excel читается") {
    MainWindow w;
    CHECK(w.open_file("checks/rep4/data/week.csv"));
    CHECK_EQ(summary(w), std::string("заявок: 8, открытых: 2, ошибок в файле: 1"));
}

TEST("файл не прочитан — таблица пуста, текст ошибки") {
    MainWindow w;
    w.open_file("checks/rep4/data/month.csv");
    CHECK(!w.open_file("checks/sum10/нет-такого.csv"));
    CHECK_EQ(summary(w), std::string("не открыть файл: checks/sum10/нет-такого.csv"));
    const QAbstractItemView* t = w.findChild<QAbstractItemView*>("requestsTable");
    CHECK(t != nullptr && t->model() != nullptr && t->model()->rowCount() == 0);
    CHECK(!w.open_file("checks/csv2/data/noheader.csv"));
    CHECK_EQ(summary(w), std::string("не файл заявок: первая строка — не заголовок"));
}
