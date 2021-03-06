cmake_minimum_required(VERSION 3.13)
project(Rose VERSION 0.1.52.52)
set(PACKAGE_CONTACT "Richard Buckley <richard.buckley@ieee.org>")
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
find_package(LocalTime REQUIRED)

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
include_directories(${LOCALTIME_INCLUDE_DIR})
include_directories(src)
include_directories(include)

list(APPEND EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 sqlite3 pthread dl rt m stdc++fs uuid)

list(APPEND ROSE_SDL_GFX_SRC
#        src/SDL2_framerate.c
#        src/SDL2_gfxPrimitives.c
#        src/SDL2_imageFilter.c
#        src/SDL2_rotozoom.c
        )

list(APPEND ROSE_SRC
        src/Application.cpp
        src/Animation.cpp
        src/AntiAliasedDrawing.cpp
        src/Button.cpp
        src/Color.cpp
        src/Frame.cpp
        src/GraphicsModel.cpp
        src/Image.cpp
        src/ImageStore.cpp
        src/Keyboard.cpp
        src/Layout.cpp
        src/Manager.cpp
        src/PointerInteractions.cpp
        src/Popup.cpp
        src/Settings.cpp
        src/StructuredTypes.cpp
        src/Surface.cpp
        src/TestWidget.cpp
        src/Text.cpp
        src/TextField.cpp
        src/Texture.cpp
        src/TimeBox.cpp
        src/TimerTick.cpp
        src/Types.cpp
        src/Utilities.cpp
        src/Visual.cpp
        src/WebCache.cpp
        src/XDGFilePaths.cpp
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_compile_definitions(VERSION=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${PROJECT_VERSION_TWEAK})
add_compile_definitions($<CONFIG>)

list(APPEND RoseLibraries rose ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${CURLPP_LIBRARIES} ${SOCI_LIBRARY}
        ${SOCI_sqlite3_PLUGIN} ${LOCALTIME_LIBRARY} ${EXTRA_LIBS})

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    add_library(rose SHARED ${ROSE_SRC} ${ROSE_SDL_GFX_SRC})
else()
    add_library(rose STATIC ${ROSE_SRC} ${ROSE_SDL_GFX_SRC})
endif()

if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug")
    add_executable(Test0 tests/test0.cpp)
    target_link_libraries(Test0 ${RoseLibraries})

    add_executable(Test2 tests/test2.cpp)
    target_link_libraries(Test2 ${RoseLibraries})

    add_executable(Test1 tests/test1.cpp)
    target_link_libraries(Test1 ${RoseLibraries})

    add_executable(IdPaths UintTests/IdPaths.cpp)
    target_link_libraries(IdPaths ${RoseLibraries})
endif()

#add_executable(Rose main.cpp)
#target_link_libraries(Rose ${RoseLibraries})
#add_dependencies(Rose rose)

#add_executable(HamChrono applications/HamChrono/HamChrono.cpp applications/HamChrono/MapProjection.cpp applications/HamChrono/MapProjection.h applications/HamChrono/ConfigDialog.cpp applications/HamChrono/ConfigDialog.h applications/HamChrono/SettingsNames.h applications/HamChrono/Plan13.cpp applications/HamChrono/Plan13.h applications/HamChrono/Ephemeris.cpp applications/HamChrono/Ephemeris.h applications/HamChrono/SatelliteDataDisplay.cpp applications/HamChrono/SatelliteDataDisplay.h src/StructuredTypes.cpp src/StructuredTypes.h)
#target_link_libraries(HamChrono ${RoseLibraries})
#add_dependencies(HamChrono rose)

add_executable(Chrono applications/Chrono/main.cpp
        applications/Chrono/MapProjection.cpp
        applications/Chrono/MapProjection.h
        applications/Chrono/SatelliteModel.cpp
        applications/Chrono/SatelliteModel.h
        applications/Chrono/Plan13.cpp
        applications/Chrono/Plan13.h
        applications/Chrono/CelestialOverlay.cpp
        applications/Chrono/CelestialOverlay.h
        applications/Chrono/GridOverlay.cpp
        applications/Chrono/GridOverlay.h)
target_link_libraries(Chrono ${RoseLibraries})

#add_executable(Life applications/ConwayLife.cpp)
#target_link_libraries(Life ${RoseLibraries})
#add_dependencies(Life rose)

#add_executable(RoseExec applications/RoseExec.cpp src0/Command.cpp)
#target_link_libraries(RoseExec stdc++fs)

#add_executable(RoseShell applications/RoseShell.cpp)
#target_link_libraries(RoseShell ${RoseLibraries})
#add_dependencies(RoseShell rose)

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
        rose
        COMPONENT ROSE_RUNTIME EXCLUDE_FROM_ALL
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

install(TARGETS
        Chrono
        COMPONENT CHRONO_RUNTIME EXCLUDE_FROM_ALL
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT ROSE_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/ChronoImages/images"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/Chrono/resources"
        COMPONENT CHRONO_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/ChronoMapsSmall/maps"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/Chrono/resources"
        COMPONENT CHRONO_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/ChronoMapsLarge/maps"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/Chrono/resources"
        COMPONENT CHRONO_LARGE
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/Rose/fonts"
        DESTINATION "${CMAKE_INSTALL_DATADIR}"
        COMPONENT ROSE_RUNTIME
        )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
        COMPONENT ROSE_DEVELOPMENT EXCLUDE_FROM_ALL
        FILES_MATCHING PATTERN "*.h")

install(FILES resources/Rose/systemd/rose.service
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
set(CPACK_DEBIAN_CHRONO_RUNTIME_PACKAGE_NAME "Chrono")
set(CPACK_COMPONENT_CHRONO_RUNTIME_DEPENDS ROSE_RUNTIME)
set(CPACK_DEBIAN_CHRONO_LARGE_PACKAGE_NAME "ChronoLarge")
set(CPACK_COMPONENT_CHRONO_LARGE_DEPENDS CHRONO_RUNTIME)
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
cpack_add_component(CHRONO_RUNTIME
        DISPLAY_NAME "Chrono"
        DESCRIPTION "A reimagining of Ham Clock using the Rose library."
        REQUIRED
        DEPENDS ROSE_RUNTIME
        )
cpack_add_component(CHRONO_LARGE
        DISPLAY_NAME "ChronoLarge"
        DESCRIPTION "Large maps for Chrono."
        REQUIRED
        DEPENDS CHRONO_RUNTIME
        )
