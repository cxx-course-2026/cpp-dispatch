// DSP-11: форма новой заявки.
#define CHECK_MAIN
#define CHECK_QT
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTest>

#include "check.hpp"
#include "new_request_dialog.hpp"

namespace {
struct Form {
    QLineEdit* house;
    QSpinBox* entrance;
    QSpinBox* flat;
    QLineEdit* phone;
    QComboBox* kind;
    QComboBox* priority;
    QLineEdit* text;
    QDialogButtonBox* box;
    bool ok() const { return house && entrance && flat && phone && kind && priority && text && box && box->button(QDialogButtonBox::Ok); }
};
Form form(NewRequestDialog& d) {
    return {d.findChild<QLineEdit*>("houseEdit"),   d.findChild<QSpinBox*>("entranceSpin"),
            d.findChild<QSpinBox*>("flatSpin"),     d.findChild<QLineEdit*>("phoneEdit"),
            d.findChild<QComboBox*>("kindCombo"),   d.findChild<QComboBox*>("priorityCombo"),
            d.findChild<QLineEdit*>("textEdit"),    d.findChild<QDialogButtonBox*>("buttonBox")};
}
}  // namespace

TEST("виджеты и значения по умолчанию") {
    NewRequestDialog d;
    const Form f = form(d);
    CHECK(f.ok());
    if (!f.ok()) return;
    CHECK_EQ(f.entrance->minimum(), 1);
    CHECK_EQ(f.entrance->maximum(), 30);
    CHECK_EQ(f.flat->minimum(), 1);
    CHECK_EQ(f.flat->maximum(), 2000);
    CHECK_EQ(f.kind->count(), 6);
    CHECK_EQ(f.kind->itemText(0).toStdString(), std::string("сантехника"));
    CHECK_EQ(f.kind->itemText(5).toStdString(), std::string("прочее"));
    CHECK_EQ(f.priority->count(), 3);
    CHECK_EQ(f.priority->currentText().toStdString(), std::string("обычная"));
    CHECK(!f.box->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST("OK — только при заполненных доме, описании и полном телефоне") {
    NewRequestDialog d;
    const Form f = form(d);
    if (!f.ok()) { CHECK(false); return; }
    QPushButton* ok = f.box->button(QDialogButtonBox::Ok);
    f.house->insert(QString::fromUtf8("Лесная 12"));
    f.text->insert(QString::fromUtf8("Течёт кран"));
    CHECK(!ok->isEnabled());
    QTest::keyClicks(f.phone, "+7916x12345");
    CHECK_EQ(f.phone->text().toStdString(), std::string("+791612345"));   // x не набирается
    CHECK(!ok->isEnabled());
    QTest::keyClicks(f.phone, "67");
    CHECK(ok->isEnabled());
    f.text->selectAll();
    QTest::keyClick(f.text, Qt::Key_Backspace);
    QTest::keyClicks(f.text, "   ");
    CHECK(!ok->isEnabled());
}

TEST("draft — поля формы") {
    NewRequestDialog d;
    const Form f = form(d);
    if (!f.ok()) { CHECK(false); return; }
    f.house->insert(QString::fromUtf8("  Садовая 5 "));
    f.entrance->setValue(3);
    f.flat->setValue(121);
    QTest::keyClicks(f.phone, "+79001112233");
    f.kind->setCurrentIndex(2);
    f.priority->setCurrentIndex(0);
    f.text->insert(QString::fromUtf8(" Застрял лифт; кричат "));
    const dispatch::Request r = d.draft();
    CHECK_EQ(r.house, std::string("Садовая 5"));
    CHECK_EQ(r.entrance, 3);
    CHECK_EQ(r.flat, 121);
    CHECK_EQ(r.phone, std::string("+79001112233"));
    CHECK_EQ(r.kind, std::string("лифт"));
    CHECK_EQ(r.priority, std::string("аварийная"));
    CHECK_EQ(r.text, std::string("Застрял лифт; кричат"));
}
