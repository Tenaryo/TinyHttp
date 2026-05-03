file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/connection.cpp
    ${CMAKE_SOURCE_DIR}/src/request.cpp
    ${CMAKE_SOURCE_DIR}/src/response.cpp
    ${CMAKE_SOURCE_DIR}/src/server.cpp
)

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE} ${LIB_SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE TinyHttp_core Threads::Threads GTest::gtest_main)
    target_compile_options(${TEST_NAME} PRIVATE -UNDEBUG)
    gtest_discover_tests(${TEST_NAME})
endforeach()
