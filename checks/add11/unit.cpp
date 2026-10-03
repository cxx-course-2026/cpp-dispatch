// DSP-11: Registry::add и добавление заявки из окна.
#define CHECK_MAIN
#define CHECK_QT
#include <QAbstractItemModel>
#include <QAction>
#include <QKeySequence>
#include <QAbstractItemView>
#include <QLabel>
#include <filesystem>
#include <sstream>

#include "check.hpp"
#include "dispatch/registry.hpp"
#include "dispatch/store.hpp"
#include "main_window.hpp"

namespace {
dispatch::Request draft(const std::string& text = "Течёт кран") {
    dispatch::Request r;
    r.house = "Лесная 12";
    r.entrance = 2;
    r.flat = 45;
    r.phone = "+79161234567";
    r.kind = "сантехника";
    r.priority = "аварийная";
    r.text = text;
    return r;
}
std::string error_of(dispatch::Registry& reg, const dispatch::Request& r) {
    try {
        reg.add(r, "2026-10-05 08:15");
    } catch (const dispatch::ParseError& e) {
        return e.what();
    }
    return "исключения нет";
}
std::string summary(MainWindow& w) {
    const QLabel* l = w.findChild<QLabel*>("summaryLabel");
    return l ? l->text().toStdString() : "<нет summaryLabel>";
}
}  // namespace

TEST("Registry::add: номер, время, состояние") {
    dispatch::Registry reg;
    CHECK_EQ(reg.add(draft(), "2026-10-05 08:15"), 1);
    CHECK_EQ(reg.add(draft(), "2026-10-05 08:20"), 2);
    const dispatch::Request* r = reg.find(2);
    CHECK(r != nullptr);
    if (r == nullptr) return;
    CHECK_EQ(r->created, std::string("2026-10-05 08:20"));
    CHECK_EQ(r->state, std::string("новая"));
    CHECK(r->master.empty() && r->done.empty());
    std::ostringstream err;
    const auto big = dispatch::Registry::load("checks/rep4/data/month.csv", err);
    dispatch::Registry copy = big;
    CHECK_EQ(copy.add(draft(), "2026-10-05 08:15"), 401);
}

TEST("Registry::add: проверка полей, реестр не меняется") {
    dispatch::Registry reg;
    dispatch::Request bad = draft();
    bad.phone = "89161234567";
    CHECK_EQ(error_of(reg, bad), std::string("телефон: не номер «89161234567»"));
    bad = draft();
    bad.kind = "газ";
    CHECK_EQ(error_of(reg, bad), std::string("неизвестный вид работ «газ»"));
    CHECK_EQ(error_of(reg, draft("")), std::string("описание: пусто"));
    CHECK_EQ(error_of(reg, draft("a\nb")), std::string("описание: перевод строки"));
    CHECK_EQ(reg.size(), std::size_t{0});
}

TEST("окно: заявка добавлена, файл сохранён") {
    std::filesystem::create_directories("build-check");
    std::filesystem::copy_file("checks/rep4/data/month.csv", "build-check/add11.csv",
                               std::filesystem::copy_options::overwrite_existing);
    MainWindow w;
    CHECK(w.open_file("build-check/add11.csv"));
    CHECK(w.add_request(draft(), "2026-10-05 08:15"));
    CHECK_EQ(summary(w), std::string("заявок: 401, открытых: 90, ошибок в файле: 0"));
    const auto* t = w.findChild<QAbstractItemView*>("requestsTable");
    CHECK(t != nullptr && t->model() && t->model()->rowCount() == 401);
    std::ostringstream err;
    const auto again = dispatch::Registry::load("build-check/add11.csv", err);
    CHECK(again.find(401) != nullptr);
    CHECK(err.str().empty());
}

TEST("окно: ошибки не портят файл") {
    std::filesystem::copy_file("checks/val3/data/bad.csv", "build-check/add11-bad.csv",
                               std::filesystem::copy_options::overwrite_existing);
    MainWindow w;
    CHECK(!w.add_request(draft(), "2026-10-05 08:15"));
    CHECK_EQ(summary(w), std::string("файл не открыт"));
    w.open_file("build-check/add11-bad.csv");
    CHECK(!w.add_request(draft(), "2026-10-05 08:15"));
    CHECK_EQ(summary(w), std::string("в файле есть ошибки — сначала исправьте их (dispatch check)"));
    CHECK(std::filesystem::file_size("build-check/add11-bad.csv") == std::filesystem::file_size("checks/val3/data/bad.csv"));
    w.open_file("checks/rep4/data/month.csv");
    CHECK(!w.add_request(draft(""), "2026-10-05 08:15"));
    CHECK_EQ(summary(w), std::string("описание: пусто"));
}

TEST("действие «Новая заявка» с клавишами Ctrl+N") {
    MainWindow w;
    const auto* a = w.findChild<QAction*>("newRequestAction");
    CHECK(a != nullptr);
    if (a != nullptr) CHECK(a->shortcut() == QKeySequence("Ctrl+N"));
}
