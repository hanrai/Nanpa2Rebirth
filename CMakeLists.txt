

cmake_minimum_required(VERSION 3.21)

project(Nanpa2Rebirth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Multimedia
    Core5Compat
    Widgets
)

set(PROJECT_SOURCES
    main.cpp
    engine.cpp
    EngineCalculator.cpp
    EngineGameCommand.cpp
    MainWindow.cpp
    config.cpp
    stack.cpp
    cps.cpp

    Engine/Engine0x3.cpp
    Engine/Engine0x4.cpp
    Engine/engine0x5.cpp
    Engine/Engine0x7.cpp
    Engine/Engine0x8.cpp
    Engine/Engine0x9.cpp
    Engine/Engine0xA.cpp
    Engine/Engine0xC.cpp
    Engine/engine0xd.cpp
    Engine/Engine0xE.cpp
    Engine/Engine0xF.cpp
    Engine/Engine0x10.cpp
    Engine/Engine0x12.cpp
    Engine/Engine0x13.cpp
    Engine/DrawString.cpp

    GameCommand/Command0x2.cpp
    GameCommand/Command0x3.cpp
    GameCommand/Command0x4.cpp
    GameCommand/Command0x5.cpp
    GameCommand/Command0x7.cpp
    GameCommand/Command0x8.cpp
    GameCommand/Command0x9.cpp
    GameCommand/Command0xA.cpp
    GameCommand/Command0xB.cpp
    GameCommand/Command0xC.cpp
    GameCommand/Command0xD.cpp
    GameCommand/Command0x12.cpp
    GameCommand/command0x13.cpp

    Files/files.cpp
    Files/DecodeArcMes.cpp
    DrawingUtil/drawingutil.cpp
    Animation/animationmanager.cpp
    Sound/soundmanager.cpp
    Decoder/decoder.cpp
)

add_executable(Nanpa2Rebirth ${PROJECT_SOURCES})

target_include_directories(Nanpa2Rebirth PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Nanpa2Rebirth PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Core5Compat
    Qt6::Widgets
)
