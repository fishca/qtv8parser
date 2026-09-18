cmake_minimum_required(VERSION 3.16)

project(qtv8parser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

#find_package(ZLIB REQUIRED)   # <-- добавить

# Добавляем zlib как подпроект
add_subdirectory(third_party/zlib-1.3.2 EXCLUDE_FROM_ALL)


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtv8parser
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        guids.h



        vcl_utils.h
        v8_constants.h
        Parse_tree.h

        Parse_tree.cpp
        MessageRegistration.h
        MessageRegistration.cpp
        V8File.h
        V8File.cpp
        VersionFile.h
        utils.cpp
        placeholder216.cpp
        APIcfBase.h
        APIcfBase.cpp
        v8api.h v8api.cpp
        globals.h
        UZLib.h
        UZlib.cpp
        metadata_base.h
        metadata_base.cpp
        metadata_objects.h
        metadata_objects.cpp
        configuration.h
        configuration.cpp
        cfparserworker.h cfparserworker.cpp
        metadata_descriptor.h
        metadata_repository.h
        metadata_repository.cpp
        cf_loader.h
        cf_loader.cpp
        configurationsession.h configurationsession.cpp
        tree_utils.h
        tree_utils.cpp
        attribute_reader.h
        attribute_reader.cpp
        attribute_metadata.h
        attribute_metadata.cpp
        resources.qrc


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qtv8parser APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qtv8parser SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qtv8parser
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#target_link_libraries(qtv8parser PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

#target_link_libraries(qtv8parser PRIVATE
#    Qt${QT_VERSION_MAJOR}::Widgets
#    #ZLIB::ZLIB                # <-- добавить
#)

# Линкуемся со статической библиотекой, которую создаст zlib
target_link_libraries(qtv8parser PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    zlibstatic  # Имя статической библиотеки, которое создает CMakeLists.txt zlib[reference:5][reference:6]
)

# Добавляем пути к заголовочным файлам zlib
target_include_directories(qtv8parser PRIVATE
    third_party/zlib-1.3.2
    ${CMAKE_CURRENT_BINARY_DIR}/third_party/zlib-1.3.2  # Для сгенерированного zconf.h
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qtv8parser)
endif()
set_target_properties(qtv8parser PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qtv8parser
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtv8parser)
endif()
