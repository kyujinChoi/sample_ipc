cmake_minimum_required(VERSION 3.5)

project(IPComm LANGUAGES CXX)

find_package(Protobuf REQUIRED)
if(protobuf_VERBOSE)
  message(STATUS "Using Protocol Buffers ${protobuf_VERSION}")
endif()
include_directories(${INCL} ${PROTOBUF_INCLUDE_DIRS})

set(EXCLUDE_PATHS
    "CMakeFiles"
)
file(GLOB_RECURSE CPP_FILES "*.cpp")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ./proto/umsg.proto)

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



    # add_executable(${PROJECT_NAME} ${CPP_FILES} app/LogManager/mainwindow.ui)
# else()
# add_executable(${PROJECT_NAME} ${CPP_FILES} ${PROTO_SRCS})
add_library(${PROJECT_NAME} SHARED ${CPP_FILES} ${PROTO_SRCS})
# endif()
# ament_target_dependencies(${PROJECT_NAME} ${AMENT_LIB})
target_link_libraries(${PROJECT_NAME} ${LIBS} ${PROTOBUF_LIBRARIES})

function(check_file_exists file result)
  if(EXISTS ${file})
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()



foreach(PROTO_HDR ${PROTO_HDRS})
    
    get_filename_component(PROTO_HDR_NAME ${PROTO_HDR} NAME)
    
    set(PROTO_HDR_NEW ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_HDR_NAME})
    check_file_exists(${PROTO_HDR_NEW} PROTO_HDR_EXISTS)
    message(STATUS "FROM ${PROTO_HDR}")
    message(STATUS "TO ${PROTO_HDR_NEW}")
    message(STATUS "PROTO_HDR_EXISTS ${PROTO_HDR_EXISTS}")
    if("${PROTO_HDR_EXISTS}" STREQUAL "FALSE")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROTO_HDR} ${PROTO_HDR_NEW}
        DEPENDS ${PROTO_HDR}
        COMMENT "Copying ${PROTO_HDR} to ${PROTO_HDR_NEW}"
    )
    list(APPEND PROTO_HDRS_NEW ${PROTO_HDR_NEW})
    endif()
endforeach()
# if(LogManager)

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