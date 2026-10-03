# Настройки компилятора проекта «Диспетчер» — такие же, как в лабораторных курса.
# Подключите в своём CMakeLists.txt сразу после project(): include(cmake/flags.cmake)
#   - C++20 без расширений компилятора;
#   - все важные предупреждения, и любое из них — ошибка сборки;
#   - в сборке Debug — санитайзеры address и undefined (кроме MSVC); выключить: -DSAN=OFF.
# Не меняйте этот файл: проверки собирают проект именно с этими настройками.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option(SAN "Санитайзеры address и undefined в сборке Debug" ON)

if(MSVC)
  add_compile_options(/W4 /WX /utf-8 /permissive- /EHsc)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror)
  if(SAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
  endif()
endif()
