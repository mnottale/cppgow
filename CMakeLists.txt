
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(cppgow_invoke STATIC src/cppgow_invoke.c)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/libcppgow.so
    DEPENDS cppgow_invoke
    COMMAND
        go build -o ${CMAKE_BINARY_DIR}/libcppgow.so -buildmode=c-shared ${CMAKE_CURRENT_SOURCE_DIR}/src/cppgow.go
    COMMAND
        cmp ${CMAKE_BINARY_DIR}/libcppgow.h ${CMAKE_CURRENT_SOURCE_DIR}/include/cppgow/cppgow.h || cp ${CMAKE_BINARY_DIR}/libcppgow.h ${CMAKE_CURRENT_SOURCE_DIR}/include/cppgow/cppgow.h
)

add_library(cppgowcxx SHARED src/cppgowcxx.cc src/router.cc ${CMAKE_BINARY_DIR}/libcppgow.so)