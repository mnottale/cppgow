
add_library(cppgow_invoke STATIC src/cppgow_invoke.c)
target_include_directories(cppgow_invoke PUBLIC include)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/libcppgow.so
    DEPENDS cppgow_invoke
    COMMAND
        go build -o ${CMAKE_BINARY_DIR}/libcppgow.so -buildmode=c-shared ${CMAKE_CURRENT_SOURCE_DIR}/src/cppgow.go
)

add_library(cppgowcxx SHARED src/cppgowcxx.cc src/router.cc ${CMAKE_BINARY_DIR}/libcppgow.so)
target_include_directories(cppgowcxx PUBLIC include)
target_include_directories(cppgowcxx PUBLIC ${CMAKE_BINARY_DIR})