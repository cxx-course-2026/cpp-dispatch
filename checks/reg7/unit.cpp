// DSP-7: модульные проверки класса Registry.
#define CHECK_MAIN
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "check.hpp"
#include "dispatch/registry.hpp"
#include "dispatch/store.hpp"

namespace {

using dispatch::Registry;
using dispatch::Request;

Request make(int id, const std::string& state, const std::string& master = "", const std::string& done = "") {
    Request r;
    r.id = id;
    r.created = "2026-10-05 08:15";
    r.house = "Лесная 12";
    r.entrance = 2;
    r.flat = 45;
    r.phone = "+79161234567";
    r.kind = "сантехника";
    r.priority = "обычная";
    r.state = state;
    r.master = master;
    r.done = done;
    r.text = "Течёт кран; капает";
    return r;
}

Registry sample() {
    return Registry({make(1, "новая"), make(2, "назначена", "Петров"), make(3, "выполнена", "Петров", "2026-10-05 10:00"),
                     make(4, "закрыта", "Иванов", "2026-10-05 11:00"), make(5, "отменена")});
}

template <class F>
std::string message(F f) {
    try {
        f();
    } catch (const std::exception& e) {
        return e.what();
    }
    return "исключения нет";
}

std::string read_bytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

}  // namespace

TEST("конструктор: повтор номера — invalid_argument") {
    CHECK_THROWS(Registry({make(1, "новая"), make(1, "новая")}), std::invalid_argument);
    const Registry r = sample();
    CHECK_EQ(r.size(), std::size_t{5});
    CHECK(r.find(3) != nullptr);
    CHECK(r.find(9) == nullptr);
}

TEST("assign: новая и назначена") {
    Registry r = sample();
    r.assign(1, "Сидоров");
    CHECK_EQ(r.find(1)->state, std::string("назначена"));
    CHECK_EQ(r.find(1)->master, std::string("Сидоров"));
    r.assign(2, "Кузнецов");
    CHECK_EQ(r.find(2)->master, std::string("Кузнецов"));
    CHECK_EQ(message([&] { r.assign(3, "Иванов"); }),
             std::string("заявка 3: нельзя назначить заявку в состоянии «выполнена»"));
    CHECK_THROWS(r.assign(1, ""), std::invalid_argument);
    CHECK_THROWS(r.assign(4, "Иванов"), dispatch::TransitionError);
}

TEST("complete: только назначенную, дата проверяется") {
    Registry r = sample();
    CHECK_EQ(message([&] { r.complete(1, "2026-10-05 12:00"); }),
             std::string("заявка 1: нельзя отметить выполненной заявку в состоянии «новая»"));
    CHECK_EQ(message([&] { r.complete(2, "завтра"); }), std::string("выполнена: не дата «завтра»"));
    CHECK_EQ(message([&] { r.complete(2, "2026-10-05 08:00"); }), std::string("выполнена раньше создания"));
    CHECK_EQ(r.find(2)->state, std::string("назначена"));     // после ошибок — без изменений
    CHECK(r.find(2)->done.empty());
    r.complete(2, "2026-10-05 12:30");
    CHECK_EQ(r.find(2)->state, std::string("выполнена"));
    CHECK_EQ(r.find(2)->done, std::string("2026-10-05 12:30"));
}

TEST("close и cancel") {
    Registry r = sample();
    r.close(3);
    CHECK_EQ(r.find(3)->state, std::string("закрыта"));
    CHECK_EQ(message([&] { r.close(2); }), std::string("заявка 2: нельзя закрыть заявку в состоянии «назначена»"));
    r.cancel(1);
    r.cancel(2);
    CHECK_EQ(r.find(2)->state, std::string("отменена"));
    CHECK_EQ(message([&] { r.cancel(4); }), std::string("заявка 4: нельзя отменить заявку в состоянии «закрыта»"));
    CHECK_THROWS(r.cancel(5), dispatch::TransitionError);
}

TEST("NotFound — наследник out_of_range") {
    Registry r = sample();
    CHECK_EQ(message([&] { r.close(42); }), std::string("заявка 42 не найдена"));
    CHECK_THROWS(r.assign(42, "Петров"), std::out_of_range);
}

TEST("save: BOM, CRLF, описание с «;», обратное чтение") {
    const std::string path = "build-check/reg7.csv";
    Registry r = sample();
    r.assign(1, "Сидоров");
    r.save(path);
    const std::string bytes = read_bytes(path);
    CHECK(bytes.rfind("\xEF\xBB\xBF", 0) == 0);
    CHECK(bytes.find("номер;создана;дом;подъезд;квартира;телефон;вид;срочность;состояние;мастер;выполнена;описание\r\n") == 3);
    CHECK(bytes.find("1;2026-10-05 08:15;Лесная 12;2;45;+79161234567;сантехника;обычная;назначена;Сидоров;;"
                     "Течёт кран; капает\r\n") != std::string::npos);
    CHECK(bytes.size() > 2 && bytes.substr(bytes.size() - 2) == "\r\n");
    CHECK(!std::filesystem::exists(path + ".tmp"));
    std::ostringstream errors;
    const Registry back = Registry::load(path, errors);
    CHECK(errors.str().empty());
    CHECK_EQ(back.size(), std::size_t{5});
    CHECK_EQ(back.find(1)->master, std::string("Сидоров"));
    CHECK_EQ(back.find(3)->done, std::string("2026-10-05 10:00"));
}

TEST("save в несуществующий каталог — FileError") {
    CHECK_THROWS(sample().save("build-check/нет/такого/каталога/x.csv"), dispatch::FileError);
}
