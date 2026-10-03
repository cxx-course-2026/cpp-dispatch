// DSP-9: модульные проверки сроков.
#define CHECK_MAIN
#include <string>

#include "check.hpp"
#include "dispatch/request.hpp"
#include "dispatch/sla.hpp"

namespace {
dispatch::Request done_request(const std::string& created, const std::string& priority, const std::string& done) {
    dispatch::Request r;
    r.id = 1;
    r.created = created;
    r.priority = priority;
    r.state = done.empty() ? "назначена" : "выполнена";
    r.master = "Петров";
    r.done = done;
    return r;
}
}  // namespace

TEST("аварийная и обычная: через границы суток, года и февраля") {
    CHECK_EQ(dispatch::deadline("2026-10-05 23:30", "аварийная"), std::string("2026-10-06 00:30"));
    CHECK_EQ(dispatch::deadline("2026-12-31 12:00", "обычная"), std::string("2027-01-01 12:00"));
    CHECK_EQ(dispatch::deadline("2028-02-28 23:30", "аварийная"), std::string("2028-02-29 00:30"));   // високосный
    CHECK_EQ(dispatch::deadline("2027-02-28 23:30", "аварийная"), std::string("2027-03-01 00:30"));
}

TEST("плановая: пять рабочих дней") {
    CHECK_EQ(dispatch::deadline("2026-10-02 10:00", "плановая"), std::string("2026-10-09 10:00"));   // пятница
    CHECK_EQ(dispatch::deadline("2026-10-03 10:00", "плановая"), std::string("2026-10-09 10:00"));   // суббота
    CHECK_EQ(dispatch::deadline("2026-10-04 23:59", "плановая"), std::string("2026-10-09 23:59"));   // воскресенье
    CHECK_EQ(dispatch::deadline("2028-02-28 09:00", "плановая"), std::string("2028-03-06 09:00"));
}

TEST("minutes_between") {
    CHECK_EQ(dispatch::minutes_between("2026-12-31 23:00", "2027-01-01 01:30"), 150LL);
    CHECK_EQ(dispatch::minutes_between("2028-02-28 10:00", "2028-03-01 10:00"), 2880LL);
    CHECK_EQ(dispatch::minutes_between("2026-10-05 10:00", "2026-10-05 10:00"), 0LL);
}

TEST("overdue") {
    CHECK(!dispatch::overdue(done_request("2026-10-05 10:00", "аварийная", "2026-10-05 11:00")));   // ровно в срок
    CHECK(dispatch::overdue(done_request("2026-10-05 10:00", "аварийная", "2026-10-05 11:01")));
    CHECK(!dispatch::overdue(done_request("2026-10-02 10:00", "плановая", "2026-10-08 18:00")));
    CHECK(dispatch::overdue(done_request("2026-10-02 10:00", "плановая", "2026-10-09 10:01")));
    CHECK(!dispatch::overdue(done_request("2026-10-02 10:00", "аварийная", "")));   // не выполнена
}
