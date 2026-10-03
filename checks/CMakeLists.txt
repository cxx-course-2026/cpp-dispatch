# Сборка проверок проекта: ваш проект — подкаталог, к нему добавляются модульные проверки
# тикетов checks/<тикет>/unit.cpp, которые линкуются с вашими библиотеками:
#   dispatch_core     — с DSP-5;
#   dispatch_gui_lib  — с DSP-10, для проверок окон (в каталоге такой проверки лежит файл qt).
# Проверки запускает checks/run.sh; руками это нужно, только если хотите посмотреть сборку:
#   cmake -S checks -B build-check && cmake --build build-check
# Не подключайте каталог checks/ из своего CMakeLists.txt.
cmake_minimum_required(VERSION 3.25)
project(dispatch_checks LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. dispatch)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/flags.cmake)
file(GLOB units CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*/unit.cpp)
foreach(unit ${units})
  get_filename_component(dir ${unit} DIRECTORY)
  get_filename_component(ticket ${dir} NAME)
  if(EXISTS ${dir}/qt)
    if(TARGET dispatch_gui_lib)
      find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
      add_executable(check_${ticket} ${unit})
      target_link_libraries(check_${ticket} PRIVATE dispatch_gui_lib Qt6::Widgets Qt6::Test)
      target_include_directories(check_${ticket} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
  elseif(TARGET dispatch_core)
    add_executable(check_${ticket} ${unit})
    target_link_libraries(check_${ticket} PRIVATE dispatch_core)
    target_include_directories(check_${ticket} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
endforeach()
