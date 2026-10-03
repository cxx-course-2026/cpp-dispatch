// DSP-10: модульные проверки окна диспетчера — таблица заявок.
#define CHECK_MAIN
#define CHECK_QT
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QBrush>
#include <QColor>

#include "check.hpp"
#include "main_window.hpp"

namespace {
// Через общий интерфейс представления и модели: подойдёт и QTableWidget, и QTableView со своей моделью.
QAbstractItemView* table(MainWindow& w) { return w.findChild<QAbstractItemView*>("requestsTable"); }
std::string cell(QAbstractItemView* t, int row, int col) {
    return t->model()->data(t->model()->index(row, col)).toString().toStdString();
}
QColor background(QAbstractItemView* t, int row, int col) {
    return t->model()->data(t->model()->index(row, col), Qt::BackgroundRole).value<QBrush>().color();
}
}  // namespace

TEST("таблица и столбцы") {
    MainWindow w;
    QAbstractItemView* t = table(w);
    CHECK(t != nullptr && t->model() != nullptr);
    if (t == nullptr || t->model() == nullptr) return;
    CHECK_EQ(t->model()->columnCount(), 7);
    const char* heads[] = {"№", "Создана", "Срочность", "Вид работ", "Состояние", "Адрес", "Мастер"};
    for (int c = 0; c < 7 && c < t->model()->columnCount(); ++c) {
        CHECK_EQ(t->model()->headerData(c, Qt::Horizontal).toString().toStdString(), std::string(heads[c]));
    }
    CHECK(t->editTriggers() == QAbstractItemView::NoEditTriggers);
    CHECK(t->selectionBehavior() == QAbstractItemView::SelectRows);
}

TEST("заявки в порядке очереди, потом остальные") {
    MainWindow w;
    CHECK(w.open_file("checks/rep4/data/month.csv"));
    QAbstractItemView* t = table(w);
    if (t == nullptr || t->model() == nullptr) return;
    CHECK_EQ(t->model()->rowCount(), 400);
    CHECK_EQ(cell(t, 0, 0), std::string("251"));
    CHECK_EQ(cell(t, 1, 0), std::string("282"));
    CHECK_EQ(cell(t, 4, 0), std::string("7"));
    CHECK_EQ(cell(t, 0, 1), std::string("2026-09-02 21:13"));
    CHECK_EQ(cell(t, 0, 2), std::string("аварийная"));
    CHECK_EQ(cell(t, 0, 3), std::string("сантехника"));
    CHECK_EQ(cell(t, 0, 4), std::string("назначена"));
    CHECK_EQ(cell(t, 0, 5), std::string("Речная 10, кв. 174"));
    CHECK_EQ(cell(t, 0, 6), std::string("Фёдоров"));
    CHECK_EQ(cell(t, 1, 6), std::string("—"));
    CHECK_EQ(cell(t, 89, 0), std::string("1"));     // первая не открытая — по номеру
    CHECK_EQ(cell(t, 89, 4), std::string("закрыта"));
}

TEST("открытые аварийные подсвечены") {
    MainWindow w;
    w.open_file("checks/rep4/data/month.csv");
    QAbstractItemView* t = table(w);
    if (t == nullptr || t->model() == nullptr || t->model()->rowCount() < 90) return;
    const QColor red(255, 220, 220);
    for (int c = 0; c < 7; ++c) {
        CHECK(background(t, 3, c) == red);
    }
    CHECK(background(t, 4, 0) != red);   // обычная
    CHECK(background(t, 89, 0) != red);
}
