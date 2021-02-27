cmake_minimum_required(VERSION 3.13)
project(Rose VERSION 0.1.48.48)
set(PACKAGE_CONTACT "Richard Buckley <richard.buckley>@ieee.org>")
include(GNUInstallDirs)

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

# Generates an RFC2822 timestamp
#
# The following variable is set:
#   RFC2822_TIMESTAMP
function(rfc2822stamp)
    execute_process(COMMAND date -R
            OUTPUT_VARIABLE RESULT)
    set(RFC2822_TIMESTAMP ${RESULT} PARENT_SCOPE)
endfunction()
rfc2822stamp()

set(CPACK_DEBIAN_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

find_program(LSB_RELEASE_CMD lsb_release)
mark_as_advanced(LSB_RELEASE_CMD)
if (LSB_RELEASE_CMD)
    execute_process(COMMAND "${LSB_RELEASE_CMD}" --codename --short
            OUTPUT_VARIABLE DISTRO_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

# generate a Debian compliant changelog
set(CHANGELOG_HEADER "${PROJECT_NAME} (${CPACK_DEBIAN_PACKAGE_VERSION}) ${DISTRO_CODENAME}\; urgency=high")
set(CHANGELOG "  * Development Release.")
# two spaces between maintainer and timestamp is NOT a mistake
set(CHANGELOG_FOOTER " -- ${PACKAGE_CONTACT}  ${RFC2822_TIMESTAMP}")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/changelog ${CHANGELOG_HEADER}\n\n${CHANGELOG}\n\n${CHANGELOG_FOOTER})

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/changelog"
    COMMENT "Compressing changelog"
)

add_custom_target(changelog ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")
add_dependencies(changelog rose)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2TTF_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
include_directories(${CURLPP_INCLUDE_DIR})
include_directories(${SOCI_INCLUDE_DIRS})
include_directories(src)
include_directories(include)

list(APPEND EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 sqlite3 pthread dl rt m stdc++fs uuid)

list(APPEND ROSE_SRC
        src/AntiAliasedDrawing.cpp
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
        src/Keyboard.cpp
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
        src/TextField.cpp
        src/Theme.cpp
        src/TimeBox.cpp
        src/Timer.cpp
        src/Utilities.cpp
        src/Slider.cpp
        src/SystemMonitor.cpp
        src/Widget.cpp
        src/XDGFilePaths.cpp
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_compile_definitions(VERSION=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${PROJECT_VERSION_TWEAK})
add_compile_definitions($<CONFIG>)

list(APPEND RoseLibraries rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES} ${SOCI_LIBRARY}
        ${SOCI_sqlite3_PLUGIN} ${EXTRA_LIBS})

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    add_library(rose SHARED ${ROSE_SRC})
else()
    add_library(rose STATIC ${ROSE_SRC})
    add_executable(Test tests/WidgetTest.cpp)
    target_link_libraries(Test ${RoseLibraries})
    add_dependencies(Test rose)
endif()


add_executable(Rose main.cpp)
target_link_libraries(Rose ${RoseLibraries})
add_dependencies(Rose rose)

add_executable(HamChrono applications/HamChrono/HamChrono.cpp applications/HamChrono/MapProjection.cpp applications/HamChrono/MapProjection.h applications/HamChrono/ConfigDialog.cpp applications/HamChrono/ConfigDialog.h applications/HamChrono/SettingsNames.h applications/HamChrono/Plan13.cpp applications/HamChrono/Plan13.h applications/HamChrono/Ephemeris.cpp applications/HamChrono/Ephemeris.h applications/HamChrono/SatelliteDataDisplay.cpp applications/HamChrono/SatelliteDataDisplay.h)
target_link_libraries(HamChrono ${RoseLibraries})
add_dependencies(HamChrono rose)

add_executable(Life applications/ConwayLife.cpp)
target_link_libraries(Life ${RoseLibraries})
add_dependencies(Life rose)

add_executable(RoseExec applications/RoseExec.cpp src/Command.cpp)
target_link_libraries(RoseExec stdc++fs)

add_executable(RoseShell applications/RoseShell.cpp)
target_link_libraries(RoseShell ${RoseLibraries})
add_dependencies(RoseShell rose)

if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        RoseConfig.cmake
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPATIBILITY SameMajorVersion
)

set_target_properties(
        rose
        PROPERTIES FRAMEWORK FALSE
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

install(TARGETS
        rose Rose Life RoseExec RoseShell HamChrono
        COMPONENT ROSE_RUNTIME EXCLUDE_FROM_ALL
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT ROSE_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/HamChrono"
        DESTINATION "${CMAKE_INSTALL_DATADIR}"
        COMPONENT ROSE_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts"
        DESTINATION "${CMAKE_INSTALL_DATADIR}"
        COMPONENT ROSE_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
        COMPONENT ROSE_DEVELOPMENT EXCLUDE_FROM_ALL
        FILES_MATCHING PATTERN "*.h")

install(FILES resources/systemd/rose.service
        DESTINATION /etc/systemd/system
        COMPONENT ROSE_RUNTIME
        )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RoseConfig.cmake
        DESTINATION share/cmake/Rose/
        COMPONENT ROSE_DEVELOPMENT EXCLUDE_FROM_ALL)

set(CPACK_DEB_COMPONENT_INSTALL 1)
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS 1)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_ROSE_DEVELOPMENT_PACKAGE_ARCHITECTURE "all")
set(CPACK_COMPONENT_ROSE_DEVELOPMENT_DEPENDS ROSE_RUNTIME)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "fonts-freefont-ttf")
set(CPACK_DEBIAN_ROSE_DEVELOPMENT_PACKAGE_DEPENDS)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_RELEASE ${PROJECT_VERSION_TWEAK})
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_DEBIAN_ROSE_RUNTIME_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_DEBIAN_ROSE_DEVELOPMENT_PACKAGE_NAME "${PROJECT_NAME}-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "hamradio")
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_URL "https://github.com/pa28/Rose")
set(CPACK_RPM_PACKAGE_SUMMARY "A lightweight GUI library targeting the Raspberry Pi.")
set(CPACK_RPM_PACKAGE_DESCRIPTION "A lightweight GUI library targeting the Raspberry Pi.")
set(CPACK_RPM_PACKAGE_VENDOR "www.hrbuckley.net")
set(CPACK_PACKAGE_CONTACT ${PACKAGE_CONTACT})
set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
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

cpack_add_component(ROSE_RUNTIME
        DISPLAY_NAME "Rose"
        DESCRIPTION "A lightweight GUI library targeting the Raspberry Pi."
        REQUIRED
        )
cpack_add_component(ROSE_DEVELOPMENT
        DISPLAY_NAME "Rose-dev"
        DESCRIPTION "A lightweight GUI library targeting the Raspberry Pi."
        REQUIRED
        DEPENDS ROSE_RUNTIME
        )
