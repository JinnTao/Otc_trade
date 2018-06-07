cmake_minimum_required(VERSION 3.8)

project(OTCTrade)

# search file
file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.cc *.qrc *.ini *.txt *.ico *.png *.rc *.aps *.conf)
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE UI_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  *.ui)
# wrap file 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group("Ui" FILES ${UI_FILES})
#message(${UI_FILES})

include_directories(base
                    core
                    ctpFiles
                    optionModel
                    oracle
                    Resources
                    Resources/Form
                    Resources/img 
                    Resources/input
                    util/easylogging
                    )

set(RCC_FILES Resources/traderqtapp.qrc)
#QT5_WRAP_UI(WRAP_FILES ${UI_FILES})
#SOURCE_GROUP("Ui" FILES ${UI_FILES} ${WRAP_FILES} )
#message(${WRAP_FILES})
add_executable(OTCTrade ${CPP_FILES} ${HEADER_FILES} ${UI_FILES})

target_link_libraries(OTCTrade
                lapack_win64_MT
                blas_win64_MT
                thosttraderapi
                thostmduserapi
                oci
                ${Qt5Core_LIBRARIES}
                ${Qt5Gui_LIBRARIES}
                ${Qt5Sql_LIBRARIES}
                ${Qt5Widgets_LIBRARIES}
)