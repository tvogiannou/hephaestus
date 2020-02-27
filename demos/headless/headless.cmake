cmake_minimum_required (VERSION 3.5.1)

function(checkTargetExists targetName)
    if (NOT TARGET ${targetName})
        message(SEND_ERROR "Cannot generate ${CMAKE_PROJECT_NAME} if target ${targetName} is not defined.")
    endif()
endfunction(checkTargetExists)

if(NOT DEFINED HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION)
    message(FATAL_ERROR "HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION needs be be defined")
endif()


add_executable(render-to-file ${CMAKE_CURRENT_LIST_DIR}/RenderOBJToImageFile.cpp)

target_include_directories(render-to-file PRIVATE "${HEPHAESTUS_EXTERNAL_DEPENDENCIES_LOCATION}/stb")

checkTargetExists(hephaestus)
checkTargetExists(common)
target_link_libraries(render-to-file PRIVATE hephaestus common ${CMAKE_DL_LIBS})