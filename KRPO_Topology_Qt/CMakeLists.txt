cmake_minimum_required(VERSION 3.10)
project(KRPO_WinAPI_Qt)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск пакетов Qt
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Добавление исходных файлов
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/drawingwidget.cpp
)

set(HEADERS
    include/mainwindow.h
    include/drawingwidget.h
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Настройка свойств для автоматического MOC
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
)

# Линковка с Qt библиотеками
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
)

# Установка путей для заголовочных файлов
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)