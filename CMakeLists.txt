cmake_minimum_required(VERSION 3.13)
project(Rose VERSION 0.1.11.13)
include(GNUInstallDirs)

add_compile_definitions(VERSION=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${PROJECT_VERSION_TWEAK})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)

option(BUILD_DOC "Build Documentation" ON)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" "${CMAKE_MODULE_PATH}")
INCLUDE(CheckCXXCompilerFlag)
FIND_FILE(BCMHOST bcm_host.h PATHS /opt/vc/include NO_DEFAULT_PATH)
IF(BCMHOST)
    add_compile_definitions(BCMHOST=1)
ELSE (BCMHOST)
    add_compile_definitions(X86HOST=1)
ENDIF (BCMHOST)

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)
find_package(SDL2TTF REQUIRED)
find_package(CURLPP REQUIRED)
find_package(Soci REQUIRED COMPONENTS sqlite3)

if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ocuments")
    # build the docs
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(Docs ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)

    else (DOXYGEN_FOUND)
        message("Doxygen needs to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif ()

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2TTF_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
include_directories(${CURLPP_INCLUDE_DIR})
include_directories(${SOCI_INCLUDE_DIRS})
include_directories(src)
include_directories(include)

list(APPEND EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 sqlite3 pthread dl rt m stdc++fs uuid)

list(APPEND ROSE_SRC
        src/Border.cpp
        src/Button.cpp
        src/Cache.cpp
        src/Color.cpp
        src/Container.cpp
        src/EventSemantics.cpp
        src/Frame.cpp
        src/Gauge.cpp
        src/ImageRepository.cpp
        src/ImageView.cpp
        src/Label.cpp
        src/LinearScale.cpp
        src/Menu.cpp
        src/PopupWindow.cpp
        src/Renderer.cpp
        src/Rose.cpp
        src/Settings.cpp
        src/ScreenMetrics.cpp
        src/ScrollArea.cpp
        src/SingleChild.cpp
        src/Surface.cpp
        src/Tab.cpp
        src/Texture.cpp
        src/Theme.cpp
        src/TimeBox.cpp
        src/Timer.cpp
        src/Utilities.cpp
        src/Slider.cpp
        src/SystemMonitor.cpp
        src/Widget.cpp
        )

list(APPEND TEST_SRC
        src/Border.cpp
        src/Button.cpp
        src/Color.cpp
        src/Container.cpp
        src/Frame.cpp
        src/Gauge.cpp
        src/ImageView.cpp
        src/Keyboard.cpp
        src/Label.cpp
        src/LinearScale.cpp
        src/Menu.cpp
        src/EventSemantics.cpp
        src/PopupWindow.cpp
        src/Renderer.cpp
        src/Rose.cpp
        src/ScreenMetrics.cpp
        src/ScrollArea.cpp
        src/SingleChild.cpp
        src/Slider.cpp
        src/Surface.cpp
        src/Texture.cpp
        src/Timer.cpp
        src/Theme.cpp
        src/Utilities.cpp
        src/Widget.cpp
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_compile_definitions(VERSION=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${PROJECT_VERSION_TWEAK})

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    add_executable(Rose main.cpp)
    add_executable(Life applications/ConwayLife.cpp)
    add_executable(RoseExec applications/RoseExec.cpp src/Command.cpp)
    add_executable(RoseShell applications/RoseShell.cpp)

    target_link_libraries(Rose rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
    target_link_libraries(Life rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
    target_link_libraries(RoseExec stdc++fs)
    target_link_libraries(RoseShell rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
else()
    add_executable(Rose main.cpp ${ROSE_SRC})
    add_executable(Life applications/ConwayLife.cpp ${ROSE_SRC})
    add_executable(Test tests/WidgetTest.cpp ${TEST_SRC})
    add_executable(RoseExec applications/RoseExec.cpp src/Command.cpp)
    add_executable(RoseShell applications/RoseShell.cpp ${ROSE_SRC})

    target_link_libraries(Rose rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
    target_link_libraries(Life ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
    target_link_libraries(Test ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
    target_link_libraries(RoseExec)
    target_link_libraries(RoseShell ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES}
            ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})
endif()

add_library(rose SHARED ${ROSE_SRC})

set_target_properties(
        rose
        PROPERTIES FRAMEWORK FALSE
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
)

get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

if ("${LIB64}" STREQUAL "TRUE")
    set(INSTALL_LIB_DIR "lib64")
else()
    set(INSTALL_LIB_DIR "lib")
endif()

install(TARGETS
        rose
        Rose Life RoseExec RoseShell
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}
        )

# ToDo: Make changes in Font.h to support multiple font paths so this can be a relative path.
install(FILES /usr/share/fonts/truetype/entypo/entypo.ttf DESTINATION /usr/share/fonts/truetype/entypo)
install(FILES resources/systemd/rose.service DESTINATION /etc/systemd/system)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "fonts-freefont-ttf")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_RELEASE ${PROJECT_VERSION_TWEAK})
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "Rose")
set(CPACK_DEBIAN_PACKAGE_SECTION "hamradio")
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_URL "https://github.com/pa28/Rose")
set(CPACK_RPM_PACKAGE_SUMMARY "A lightweight GUI library targeting the Raspberry Pi.")
set(CPACK_RPM_PACKAGE_DESCRIPTION "A lightweight GUI library targeting the Raspberry Pi.")
set(CPACK_RPM_PACKAGE_VENDOR "www.hrbuckley.net")
set(CPACK_PACKAGE_CONTACT "Richard Buckley")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_EXECUTABLES Rose Life RoseExec RoseShell)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
        /usr/local
        /usr/local/bin
        /usr/local/share
        /usr/local/include
        /usr/lib/systemd
        /usr/lib/systemd/system
        )
SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
include(CPack)
