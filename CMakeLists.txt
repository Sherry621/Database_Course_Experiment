cmake_minimum_required(VERSION 3.20)

project(GenealogySystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    src/main.cpp
    src/db/DatabaseManager.cpp
    src/db/UserDao.cpp
    src/db/GenealogyDao.cpp
    src/db/MemberDao.cpp
    src/db/RelationDao.cpp
    src/service/AuthService.cpp
    src/service/DashboardService.cpp
    src/service/TreeService.cpp
    src/ui/GenealogyDialog.cpp
    src/ui/LoginWindow.cpp
    src/ui/MainWindow.cpp
    src/ui/MemberDialog.cpp
)

set(HEADERS
    src/model/User.h
    src/model/Genealogy.h
    src/model/Member.h
    src/model/RelationPath.h
    src/db/DatabaseManager.h
    src/db/UserDao.h
    src/db/GenealogyDao.h
    src/db/MemberDao.h
    src/db/RelationDao.h
    src/service/AuthService.h
    src/service/DashboardService.h
    src/service/TreeService.h
    src/ui/GenealogyDialog.h
    src/ui/LoginWindow.h
    src/ui/MainWindow.h
    src/ui/MemberDialog.h
)

add_executable(GenealogySystem ${SOURCES} ${HEADERS})

target_include_directories(GenealogySystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(GenealogySystem PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
