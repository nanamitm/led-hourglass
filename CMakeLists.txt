cmake_minimum_required(VERSION 3.21)
project(LedHourglass VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Sensors)
qt_standard_project_setup()

qt_add_executable(LedHourglass
    src/main.cpp
    src/pixel_dust.cpp
    src/hourglass_sim.cpp
    src/hourglass_item.cpp
    src/gravity_source.cpp
    $<$<PLATFORM_ID:Windows>:resources/app.rc>
)

qt_add_qml_module(LedHourglass
    URI LedHourglass
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/HourglassView.qml
    SOURCES
        src/hourglass_sim.h
        src/hourglass_item.h
)

target_include_directories(LedHourglass PRIVATE src)

target_link_libraries(LedHourglass PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Sensors)

set_target_properties(LedHourglass PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "led-hourglass"
)

if(ANDROID)
    add_link_options(-Wl,-z,max-page-size=16384)
    set_target_properties(LedHourglass PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_TARGET_SDK_VERSION 35
        QT_ANDROID_MIN_SDK_VERSION    28
    )
    set_property(TARGET LedHourglass APPEND PROPERTY
        QT_ANDROID_PACKAGE_NAME "net.nanamitm.ledhourglass"
    )
endif()
