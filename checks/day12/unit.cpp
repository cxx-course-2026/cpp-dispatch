// DSP-12: заявки по дням и график.
#define CHECK_MAIN
#define CHECK_QT
#include <QImage>
#include <sstream>

#include "check.hpp"
#include "day_chart.hpp"
#include "dispatch/registry.hpp"
#include "dispatch/stats.hpp"
#include "main_window.hpp"

namespace {
dispatch::Request make(int id, const std::string& created) {
    dispatch::Request r;
    r.id = id;
    r.created = created;
    r.house = "Лесная 1";
    r.entrance = 1;
    r.flat = 1;
    r.phone = "+79161234567";
    r.kind = "лифт";
    r.priority = "обычная";
    r.state = "новая";
    r.text = "…";
    return r;
}
using Days = std::vector<std::pair<std::string, int>>;
}  // namespace

TEST("per_day: подряд, с нулями, через границу месяца") {
    const dispatch::Registry reg({make(1, "2026-09-30 10:00"), make(2, "2026-10-02 09:00"), make(3, "2026-09-30 23:59"),
                                  make(4, "2026-10-02 00:00")});
    const Days want = {{"2026-09-30", 2}, {"2026-10-01", 0}, {"2026-10-02", 2}};
    CHECK(dispatch::per_day(reg) == want);
    CHECK(dispatch::per_day(dispatch::Registry()).empty());
}

TEST("график показывает дни открытого файла") {
    MainWindow w;
    w.open_file("checks/rep4/data/month.csv");
    const auto* chart = w.findChild<DayChart*>("dayChart");
    CHECK(chart != nullptr);
    if (chart == nullptr) return;
    const Days want = {{"2026-09-01", 126}, {"2026-09-02", 140}, {"2026-09-03", 134}};
    CHECK(chart->days() == want);
    w.open_file("checks/rep4/data/week.csv");
    CHECK(chart->days() == Days({{"2026-09-01", 8}}));
}

TEST("график рисует столбики") {
    MainWindow w;
    w.open_file("checks/rep4/data/month.csv");
    auto* chart = w.findChild<DayChart*>("dayChart");
    if (chart == nullptr) { CHECK(false); return; }
    chart->resize(300, 150);
    QImage img(chart->size(), QImage::Format_RGB32);
    img.fill(Qt::white);
    chart->render(&img);
    int colored = 0;
    for (int y = 0; y < img.height(); y += 3) {
        for (int x = 0; x < img.width(); x += 3) {
            const QColor c = img.pixelColor(x, y);
            if (c.saturation() > 60) ++colored;
        }
    }
    CHECK(colored > 200);   // заметная часть — цветные столбики, а не пустой фон
}
