# Сборка и проверки проекта «Диспетчер».
#   make              собрать проект в build/
#   make check        проверки всех тикетов
#   make check-tz1    одна проверка
#   make update       забрать новые тикеты и проверки из шаблона курса

all:
	@cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug $(CMAKE_ARGS) >/dev/null
	@cmake --build build

check:
	@bash checks/run.sh

check-%:
	@bash checks/run.sh $*

update:
	@bash checks/update.sh

clean:
	rm -rf build build-check

.PHONY: all check update clean
