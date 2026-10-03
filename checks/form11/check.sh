# DSP-11: форма новой заявки (Qt Designer, проверка ввода).
[ -f src/gui/new_request_dialog.ui ] || fail "нет формы src/gui/new_request_dialog.ui — её собирают в Qt Designer"
qt_tests check_form11
