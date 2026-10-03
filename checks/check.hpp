// Маленькие модульные тесты курса «C++ и Qt» — чтобы не тянуть внешнюю библиотеку.
//
//   #define CHECK_MAIN          // в одном файле с тестами: там будет main()
//   #define CHECK_QT            // и ещё это — для тестов на Qt: main создаст QApplication
//   #include "check.hpp"
//
//   TEST("сумма двух чисел") {
//       CHECK(add(2, 2) == 4);
//       CHECK_EQ(add(2, 2), 4);             // при ошибке покажет оба значения
//       CHECK_NEAR(average({1, 2}), 1.5, 1e-9);
//       CHECK_THROWS(parse("x"), std::invalid_argument);
//   }
//
// Программа печатает «✓ имя» или «✗ имя» и строки непрошедших проверок,
// код возврата 0 — все тесты прошли. Не меняйте этот файл.
#pragma once

#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace check {

struct Case {
    const char* name;
    void (*fn)();
};

inline std::vector<Case>& cases() {
    static std::vector<Case> all;
    return all;
}

inline int& failures() {
    static int n = 0;
    return n;
}

struct Register {
    Register(const char* name, void (*fn)()) { cases().push_back({name, fn}); }
};

template <class T>
std::string show(const T& value) {
    if constexpr (std::is_convertible_v<const T&, std::string_view>) {
        return "\"" + std::string(std::string_view(value)) + "\"";
    } else if constexpr (requires(std::ostream& out) { out << value; }) {
        std::ostringstream out;
        out << value;
        return out.str();
    } else {
        return "<значение без operator<<>";
    }
}

inline void report(const char* file, int line, const std::string& what) {
    ++failures();
    std::cout << "      " << file << ":" << line << ": " << what << "\n";
}

inline int run_all() {
    int failed = 0;
    for (const Case& c : cases()) {
        const int before = failures();
        std::ostringstream log;
        auto* old = std::cout.rdbuf(log.rdbuf());
        try {
            c.fn();
        } catch (const std::exception& e) {
            report("тест", 0, std::string("необработанное исключение: ") + e.what());
        } catch (...) {
            report("тест", 0, "необработанное исключение неизвестного типа");
        }
        std::cout.rdbuf(old);
        if (failures() == before) {
            std::cout << "✓ " << c.name << "\n";
        } else {
            ++failed;
            std::cout << "✗ " << c.name << "\n" << log.str();
        }
    }
    std::cout << (cases().size() - failed) << " из " << cases().size() << " тестов пройдено\n";
    return failed == 0 ? 0 : 1;
}

}  // namespace check

#define CHECK_CAT2_(a, b) a##b
#define CHECK_CAT_(a, b) CHECK_CAT2_(a, b)
#define TEST_IMPL_(fn, reg, name) \
    static void fn();             \
    static const check::Register reg(name, fn); \
    static void fn()
#define TEST(name) TEST_IMPL_(CHECK_CAT_(check_test_, __LINE__), CHECK_CAT_(check_reg_, __LINE__), name)

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) check::report(__FILE__, __LINE__, "не выполнено: " #expr); \
    } while (0)

#define CHECK_EQ(actual, expected)                                                        \
    do {                                                                                  \
        const auto check_a_ = (actual);   /* копия: (actual) может ссылаться во временный объект */ \
        const auto check_e_ = (expected);                                                 \
        if (!(check_a_ == check_e_))                                                      \
            check::report(__FILE__, __LINE__,                                             \
                          std::string(#actual " — получено ") + check::show(check_a_) +   \
                              ", ожидалось " + check::show(check_e_));                    \
    } while (0)

#define CHECK_NEAR(actual, expected, eps)                                                 \
    do {                                                                                  \
        const double check_a_ = (actual);                                                 \
        const double check_e_ = (expected);                                               \
        if (!(std::fabs(check_a_ - check_e_) <= (eps)))                                   \
            check::report(__FILE__, __LINE__,                                             \
                          std::string(#actual " — получено ") + check::show(check_a_) +   \
                              ", ожидалось " + check::show(check_e_));                    \
    } while (0)

#define CHECK_THROWS(expr, Type)                                                         \
    do {                                                                                 \
        bool check_ok_ = false;                                                          \
        std::string check_other_ = "исключения не было";                                 \
        try {                                                                            \
            (void)(expr);                                                                \
        } catch (const Type&) {                                                          \
            check_ok_ = true;                                                            \
        } catch (const std::exception& e) {                                              \
            check_other_ = std::string("было другое исключение: ") + e.what();           \
        } catch (...) {                                                                  \
            check_other_ = "было исключение другого типа";                               \
        }                                                                                \
        if (!check_ok_)                                                                  \
            check::report(__FILE__, __LINE__,                                            \
                          std::string(#expr " должно бросить " #Type ", а ") + check_other_); \
    } while (0)

#ifdef CHECK_MAIN
#ifdef CHECK_QT
// Тесты на Qt: виджетам нужно приложение. Окна рисуются без экрана: QT_QPA_PLATFORM=offscreen.
#include <QApplication>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    return check::run_all();
}
#else
int main() { return check::run_all(); }
#endif
#endif
