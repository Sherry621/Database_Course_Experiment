cmake_minimum_required(VERSION 3.20)

project(GenealogySystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    main.cpp
    db/DatabaseManager.cpp
    db/UserDao.cpp
    db/GenealogyDao.cpp
    db/MemberDao.cpp
    db/RelationDao.cpp
    service/AuthService.cpp
    service/DashboardService.cpp
    service/TreeService.cpp
    ui/LoginWindow.cpp
    ui/MainWindow.cpp
    ui/MemberDialog.cpp
)

set(HEADERS
    model/User.h
    model/Genealogy.h
    model/Member.h
    model/RelationPath.h
    db/DatabaseManager.h
    db/UserDao.h
    db/GenealogyDao.h
    db/MemberDao.h
    db/RelationDao.h
    service/AuthService.h
    service/DashboardService.h
    service/TreeService.h
    ui/LoginWindow.h
    ui/MainWindow.h
    ui/MemberDialog.h
)

add_executable(GenealogySystem ${SOURCES} ${HEADERS})

target_include_directories(GenealogySystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(GenealogySystem PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
