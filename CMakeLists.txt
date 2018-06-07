cmake_minimum_required(VERSION 3.8)

project(OTCTrade)

#below for QT
set(CMAKE_PREFIX_PATH "D:\\Qt\\Qt5.2.0\\5.2.0\\msvc2012")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

#target_link_libraries(OTCTrade )

message(${Qt5Core_INCLUDE_DIRS})
message(${Qt5Gui_INCLUDE_DIRS})
message(${Qt5Sql_INCLUDE_DIRS})
message(${Qt5Widgets_INCLUDE_DIRS})

message(${Qt5Sql_DEFINITIONS})
message(${Qt5Sql_COMPILE_DEFINITIONS})

message(${Qt5Core_LIBRARIES})
message(${Qt5Core_DEFINITIONS})
message(${Qt5Core_COMPILE_DEFINITIONS})

#message(${Qt5_LIBRARY_DIRS})

#
include_directories(third/instantclient_11_2/sdk/include
                    third/armadillo

)
link_directories(third/instantclient_11_2/sdk/lib/msvc
    lib
    lib/lib_win64
)

add_subdirectory(src)

