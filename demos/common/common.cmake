cmake_minimum_required (VERSION 3.5.1)

if(NOT DEFINED HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION)
    message(FATAL_ERROR "HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION needs be be defined")
endif()

# add sources
set(COMMON_SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/src/AxisAlignedBoundingBox.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Matrix3.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Matrix4.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Vector3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Vector4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/CommonUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/MeshUtils.cpp
	${CMAKE_CURRENT_LIST_DIR}/include/common/AxisAlignedBoundingBox.h
	${CMAKE_CURRENT_LIST_DIR}/include/common/Matrix3.h
	${CMAKE_CURRENT_LIST_DIR}/include/common/Matrix4.h
	${CMAKE_CURRENT_LIST_DIR}/include/common/Vector3.h
    ${CMAKE_CURRENT_LIST_DIR}/include/common/Vector4.h
    ${CMAKE_CURRENT_LIST_DIR}/include/common/Camera.h
    ${CMAKE_CURRENT_LIST_DIR}/include/common/CommonUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/common/MeshUtils.h)
set(COMMON_PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(common STATIC ${COMMON_SOURCE_FILES})
target_include_directories(common PUBLIC ${COMMON_PUBLIC_HEADERS_DIR})

# header only dependencies
target_include_directories(common PRIVATE "${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/stb")
target_include_directories(common PRIVATE "${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/tiny_obj_loader")

target_link_libraries(common PUBLIC hephaestus)