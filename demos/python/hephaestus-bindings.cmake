cmake_minimum_required (VERSION 3.5.1)

function(checkTargetExists targetName)
    if (NOT TARGET ${targetName})
        message(SEND_ERROR "Cannot generate ${CMAKE_PROJECT_NAME} if target ${targetName} is not defined.")
    endif()
endfunction(checkTargetExists)

if(NOT DEFINED HEPHAESTUS_PYBIND11_HEADERS_DIR)
    message(FATAL_ERROR "HEPHAESTUS_PYBIND11_HEADERS_DIR needs be be defined")
endif()


if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -ldl -lpthread -fpic")   # rtti for pybind11
endif()
    
add_subdirectory(${HEPHAESTUS_PYBIND11_HEADERS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/pybind11-build)

# create pybind11 module
pybind11_add_module(hephaestus_bindings 
    ${CMAKE_CURRENT_LIST_DIR}/module.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HephaestusBindingsUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/module.h)

# link with targets created outside the script
checkTargetExists(hephaestus)
checkTargetExists(common)
target_link_libraries(hephaestus_bindings
            PRIVATE hephaestus common)