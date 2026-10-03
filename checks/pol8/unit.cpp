// DSP-8: модульные проверки мастеров, правил назначения и очереди.
#define CHECK_MAIN
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "check.hpp"
#include "dispatch/masters.hpp"
#include "dispatch/queue.hpp"
#include "dispatch/registry.hpp"

namespace {

using dispatch::Master;
using dispatch::Registry;
using dispatch::Request;

Request make(int id, const std::string& kind, const std::string& state, const std::string& master = "",
             const std::string& priority = "обычная", const std::string& created = "2026-10-05 08:15") {
    Request r;
    r.id = id;
    r.created = created;
    r.house = "Лесная 12";
    r.entrance = 1;
    r.flat = 1;
    r.phone = "+79161234567";
    r.kind = kind;
    r.priority = priority;
    r.state = state;
    r.master = master;
    r.text = "…";
    return r;
}

const std::vector<Master> kMasters = {
    {"Петров", "сантехника"}, {"Сидоров", "сантехника"}, {"Морозов", "лифт"},
    {"Васильев", "универсал"}, {"Фёдоров", "универсал"},
};

std::string chosen(const dispatch::AssignPolicy& p, const Request& r, const Registry& reg) {
    const Master* m = p.choose(r, kMasters, reg);
    return m == nullptr ? "никто" : m->name;
}

std::string parse_error(const std::string& text) {
    std::istringstream in(text);
    try {
        (void)dispatch::read_masters(in);
    } catch (const dispatch::ParseError& e) {
        return e.what();
    }
    return "исключения нет";
}

}  // namespace

TEST("read_masters: BOM, \\r\\n, специальности") {
    std::istringstream in("\xEF\xBB\xBFмастер;специальность\r\nПетров;сантехника\r\nВасильев;универсал\r\n");
    const auto ms = dispatch::read_masters(in);
    CHECK_EQ(ms.size(), std::size_t{2});
    if (ms.size() == 2) {
        CHECK_EQ(ms[1].name, std::string("Васильев"));
        CHECK_EQ(ms[1].specialty, std::string("универсал"));
    }
}

TEST("read_masters: ошибки файла") {
    CHECK_EQ(parse_error("фамилия;кто\nПетров;лифт\n"), std::string("мастера: первая строка — не заголовок"));
    CHECK_EQ(parse_error("мастер;специальность\nПетров;лифт\n;лифт\n"), std::string("мастера, строка 3: пустая фамилия"));
    CHECK_EQ(parse_error("мастер;специальность\nПетров;газовщик\n"),
             std::string("мастера, строка 2: неизвестная специальность «газовщик»"));
    CHECK_EQ(parse_error("мастер;специальность\nПетров;лифт\nПетров;уборка\n"),
             std::string("мастера, строка 3: мастер «Петров» уже есть"));
}

TEST("workload — только заявки «назначена»") {
    const Registry reg({make(1, "лифт", "назначена", "Морозов"), make(2, "лифт", "выполнена", "Морозов"),
                        make(3, "лифт", "назначена", "Морозов")});
    CHECK_EQ(dispatch::workload(reg, "Морозов"), 2);
    CHECK_EQ(dispatch::workload(reg, "Петров"), 0);
}

TEST("SpecialtyPolicy: свободный специалист, при равенстве — по фамилии") {
    const Registry reg({make(1, "сантехника", "назначена", "Петров")});
    const dispatch::SpecialtyPolicy p;
    CHECK_EQ(chosen(p, make(9, "сантехника", "новая"), reg), std::string("Сидоров"));
    const Registry empty;
    CHECK_EQ(chosen(p, make(9, "сантехника", "новая"), empty), std::string("Петров"));
}

TEST("SpecialtyPolicy: специалисты заняты — свободный универсал") {
    const Registry reg({make(1, "лифт", "назначена", "Морозов"), make(2, "лифт", "назначена", "Морозов"),
                        make(3, "лифт", "назначена", "Морозов"), make(4, "уборка", "назначена", "Васильев")});
    const dispatch::SpecialtyPolicy p;
    CHECK_EQ(chosen(p, make(9, "лифт", "новая"), reg), std::string("Фёдоров"));
}

TEST("SpecialtyPolicy: заняты все — наименее загруженный из специалистов и универсалов") {
    std::vector<Request> rs;
    int id = 1;
    for (const char* name : {"Морозов", "Морозов", "Морозов", "Морозов", "Васильев", "Васильев", "Васильев",
                             "Фёдоров", "Фёдоров", "Фёдоров", "Фёдоров", "Фёдоров"}) {
        rs.push_back(make(id++, "прочее", "назначена", name));
    }
    const Registry reg(rs);
    const dispatch::SpecialtyPolicy p;
    CHECK_EQ(chosen(p, make(99, "лифт", "новая"), reg), std::string("Васильев"));
    CHECK_EQ(chosen(p, make(99, "электрика", "новая"), reg), std::string("Васильев"));   // электриков нет вовсе
}

TEST("LeastLoadedPolicy и make_policy") {
    const Registry reg({make(1, "лифт", "назначена", "Васильев")});
    const auto least = dispatch::make_policy("least");
    CHECK_EQ(chosen(*least, make(9, "лифт", "новая"), reg), std::string("Морозов"));
    const auto spec = dispatch::make_policy("specialty");
    CHECK_EQ(chosen(*spec, make(9, "лифт", "новая"), reg), std::string("Морозов"));
    CHECK_THROWS(dispatch::make_policy("nearest"), std::invalid_argument);
    const std::vector<Master> none;
    CHECK(least->choose(make(9, "лифт", "новая"), none, reg) == nullptr);
}

TEST("очередь: аварийные первыми, потом по времени и номеру") {
    const Registry reg({make(1, "лифт", "новая", "", "плановая", "2026-10-01 08:00"),
                        make(2, "лифт", "назначена", "Морозов", "аварийная", "2026-10-05 09:00"),
                        make(3, "лифт", "закрыта", "Морозов", "аварийная", "2026-10-01 07:00"),
                        make(4, "лифт", "новая", "", "обычная", "2026-10-02 08:00"),
                        make(5, "лифт", "новая", "", "аварийная", "2026-10-05 09:00")});
    const auto q = dispatch::open_queue(reg);
    std::string order;
    for (const Request& r : q) order += std::to_string(r.id);
    CHECK_EQ(order, std::string("2541"));
    CHECK(dispatch::compare_urgency(make(1, "лифт", "новая", "", "аварийная"), make(2, "лифт", "новая", "", "плановая")) < 0);
}
