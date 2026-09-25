cmake_minimum_required(VERSION 3.16)
project(Cajero CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

# El dominio: Dinero y Cuenta (solo cabeceras)
add_library(cajero INTERFACE)
target_include_directories(cajero INTERFACE src)

# ── Bucle interno: tests unitarios con GoogleTest (Windows y Linux) ──
enable_testing()
add_executable(cajero_tests
    tests/dinero_test.cpp
    tests/cuenta_test.cpp)
target_link_libraries(cajero_tests PRIVATE cajero GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(cajero_tests)

# ── Bucle externo: pasos de Cucumber (solo si cucumber-cpp está instalado) ──
find_package(nlohmann_json QUIET)   # cucumber-cpp lo necesita y su config no lo busca
find_package(CucumberCpp QUIET)

if(CucumberCpp_FOUND)
    add_executable(cajero_steps
        features/step_definitions/cuenta_steps.cpp
        features/step_definitions/transferencia_steps.cpp)
    target_link_libraries(cajero_steps PRIVATE cajero CucumberCpp::cucumber-cpp GTest::gtest)
else()
    message(STATUS "cucumber-cpp no encontrado: solo se compilan los tests unitarios")
endif()
