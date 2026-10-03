// DSP-5: модульные проверки курса против вашей библиотеки dispatch_core.
#define CHECK_MAIN
#include <sstream>
#include <string>
#include <vector>

#include "check.hpp"
#include "dispatch/request.hpp"
#include "dispatch/store.hpp"
#include "dispatch/text.hpp"

namespace {

const std::string kHeader =
    "номер;создана;дом;подъезд;квартира;телефон;вид;срочность;состояние;мастер;выполнена;описание";
const std::string kLine =
    "17;2026-10-05 08:15;Лесная 12;2;45;+79161234567;сантехника;аварийная;выполнена;Петров;2026-10-05 09:02;"
    "Течёт стояк; вода в подъезде";

std::string error_of(const std::string& line) {
    try {
        (void)dispatch::parse_request(line);
    } catch (const dispatch::ParseError& e) {
        return e.what();
    }
    return "исключения нет";
}

}  // namespace

TEST("parse_request: все поля правильной строки") {
    const dispatch::Request r = dispatch::parse_request(kLine);
    CHECK_EQ(r.id, 17);
    CHECK_EQ(r.created, std::string("2026-10-05 08:15"));
    CHECK_EQ(r.house, std::string("Лесная 12"));
    CHECK_EQ(r.entrance, 2);
    CHECK_EQ(r.flat, 45);
    CHECK_EQ(r.phone, std::string("+79161234567"));
    CHECK_EQ(r.kind, std::string("сантехника"));
    CHECK_EQ(r.priority, std::string("аварийная"));
    CHECK_EQ(r.state, std::string("выполнена"));
    CHECK_EQ(r.master, std::string("Петров"));
    CHECK_EQ(r.done, std::string("2026-10-05 09:02"));
    CHECK_EQ(r.text, std::string("Течёт стояк; вода в подъезде"));
}

TEST("parse_request: тексты ошибок из DSP-3") {
    CHECK_EQ(error_of("17;2026-10-05"), std::string("полей 2, нужно 12"));
    CHECK_EQ(error_of("0" + kLine.substr(2)), std::string("номер: не целое больше нуля «0»"));
    std::string bad_kind = kLine;
    bad_kind.replace(bad_kind.find("сантехника"), std::string("сантехника").size(), "газ");
    CHECK_EQ(error_of(bad_kind), std::string("неизвестный вид работ «газ»"));
    std::string bad_phone = kLine;
    bad_phone.replace(bad_phone.find("+79161234567"), 12, "89161234567");
    CHECK_EQ(error_of(bad_phone), std::string("телефон: не номер «89161234567»"));
    CHECK_THROWS(dispatch::parse_request(""), std::runtime_error);   // ParseError — наследник runtime_error
}

TEST("read_requests из потока: BOM, \\r\\n, ошибка строки") {
    std::istringstream in("\xEF\xBB\xBF" + kHeader + "\r\n" + kLine + "\r\n" + "5;плохо\r\n\r\n");
    std::ostringstream errors;
    const auto all = dispatch::read_requests(in, errors);
    CHECK_EQ(all.size(), std::size_t{1});
    if (!all.empty()) {
        CHECK_EQ(all[0].text, std::string("Течёт стояк; вода в подъезде"));
    }
    CHECK_EQ(errors.str(), std::string("строка 3: полей 2, нужно 12\n"));
}

TEST("read_requests: не тот заголовок — FileError") {
    std::istringstream in("id;name\n1;x\n");
    std::ostringstream errors;
    CHECK_THROWS(dispatch::read_requests(in, errors), dispatch::FileError);
}

TEST("load_requests: нет файла — FileError") {
    std::ostringstream errors;
    CHECK_THROWS(dispatch::load_requests("checks/lib5/нет-такого.csv", errors), dispatch::FileError);
}

TEST("find_request") {
    std::istringstream in(kHeader + "\n" + kLine + "\n");
    std::ostringstream errors;
    const auto all = dispatch::read_requests(in, errors);
    const dispatch::Request* hit = dispatch::find_request(all, 17);
    CHECK(hit != nullptr);
    if (hit != nullptr) {
        CHECK_EQ(hit->flat, 45);
    }
    CHECK(dispatch::find_request(all, 18) == nullptr);
}

TEST("text: символы UTF-8, регистр, ширина") {
    CHECK_EQ(dispatch::utf8_length("сантехника"), std::size_t{10});
    CHECK_EQ(dispatch::utf8_lower("ЛЕСНАЯ Ёлка ABC"), std::string("лесная ёлка abc"));
    CHECK_EQ(dispatch::pad_right("лифт", 6), std::string("лифт  "));
    CHECK_EQ(dispatch::pad_left("42", 5), std::string("   42"));
    CHECK_EQ(dispatch::pad_right("сантехника", 4), std::string("сантехника"));
}
