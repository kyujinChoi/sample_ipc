cmake_minimum_required(VERSION 3.5)

project(ipc_client LANGUAGES CXX)

include_directories(${INCL})

set(EXCLUDE_PATHS
    "CMakeFiles"
)
file(GLOB_RECURSE CPP_FILES "*.cpp")

foreach(TMP_PATH ${CPP_FILES})
    foreach(EXCLUDE_PATH ${EXCLUDE_PATHS})
        string(FIND ${TMP_PATH} ${EXCLUDE_PATH} RESULT)

        if(NOT ${RESULT} EQUAL -1)
            list(REMOVE_ITEM CPP_FILES ${TMP_PATH})
        endif()
    endforeach(EXCLUDE_PATH)
endforeach(TMP_PATH)

foreach(TMP_PATH ${CPP_FILES})
message(STATUS "CPP_FILES : ${CPP_FILES}")
endforeach(TMP_PATH)

# if(LogManager)
    # add_executable(${PROJECT_NAME} ${CPP_FILES} app/LogManager/mainwindow.ui)
# else()
add_executable(${PROJECT_NAME} ${CPP_FILES})
# add_library(${PROJECT_NAME} SHARED ${CPP_FILES})
# endif()
# ament_target_dependencies(${PROJECT_NAME} ${AMENT_LIB})
target_link_libraries(${PROJECT_NAME} ${LIBS})
# add_dependencies(${PROJECT_NAME} BagWriter LidarDriver)
# install(TARGETS
# ${PROJECT_NAME}
#   DESTINATION lib/${PROJECT_NAME}
# )
# add_custom_command(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/www
#     ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/www)