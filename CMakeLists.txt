
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(cppgow_invoke STATIC src/cppgow_invoke.c)

add_custom_command(
    OUTPUT libcppgow.so
    DEPENDS cppgow_invoke
    COMMAND
        go build -o ${CMAKE_BINARY_DIR}/libcppgow.so -buildmode=c-shared ${CMAKE_CURRENT_SOURCE_DIR}/src/cppgow.go
    COMMAND
        cp ${CMAKE_CURRENT_BINARY_DIR}/cppgow.h ${CMAKE_CURRENT_SOURCE_DIR}/include/cppgow
)

add_library(cppgowcxx SHARED src/cppgowcxx.cc src/router.cc libcppgow.so)