cmake_minimum_required(VERSION 3.16)

# Remove it.
# list(APPEND CMAKE_PREFIX_PATH "F:\\dev\\programs\\Qt\\Tools\\QtCreator\\bin")
list(APPEND CMAKE_PREFIX_PATH "F:\\dev\\programs\\Qt\\Tools\\QtCreator\\lib\\cmake\\QtCreator")

project(QLP)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QtCreator REQUIRED COMPONENTS Core)
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

# Add a CMake option that enables building your plugin with tests.
# You don't want your released plugin binaries to contain tests,
# so make that default to 'NO'.
# Enable tests by passing -DWITH_TESTS=ON to CMake.
option(WITH_TESTS "Builds with tests" NO)

if(WITH_TESTS)
  # Look for QtTest
  find_package(Qt6 REQUIRED COMPONENTS Test)

  # Tell CMake functions like add_qtc_plugin about the QtTest component.
  set(IMPLICIT_DEPENDS Qt::Test)

  # Enable ctest for auto tests.
  enable_testing()
endif()

add_qtc_plugin(QLP
  PLUGIN_DEPENDS
    QtCreator::Core
  DEPENDS
    Qt::Widgets
    Qt::Network
    QtCreator::ExtensionSystem
    QtCreator::Utils
  SOURCES
    # .github/workflows/build_cmake.yml
    # .github/workflows/README.md
    README.md
    qlp.cpp
    qlpconstants.h
    qlptr.h
)

target_sources(QLP
  PRIVATE
    src/ui/chatdockwidget.h src/ui/chatdockwidget.cpp
    src/ui/chatmessagewidget.h src/ui/chatmessagewidget.cpp
    src/ui/llmoptionspage.h src/ui/llmoptionspage.cpp
    src/ui/typingindicatorwidget.h src/ui/typingindicatorwidget.cpp

    src/providers/base/llmprovider.h src/providers/base/llmprovider.cpp
    src/providers/ollama/ollamaprovider.h src/providers/ollama/ollamaprovider.cpp

    src/llmmanager.h src/llmmanager.cpp

    src/settings/llmsettings.h src/settings/llmsettings.cpp
)

# Enable the Run button in Qt Creator
get_target_property(QtCreatorCorePath QtCreator::Core LOCATION)
find_program(QtCreatorExecutable
  NAMES
    qtcreator "Qt Creator"
  PATHS
    "${QtCreatorCorePath}/../../../bin"
    "${QtCreatorCorePath}/../../../MacOS"
  NO_DEFAULT_PATH
)
if (QtCreatorExecutable)
  add_custom_target(RunQtCreator
    COMMAND ${QtCreatorExecutable} -pluginpath $<TARGET_FILE_DIR:QLP>
    DEPENDS QLP
  )
  set_target_properties(RunQtCreator PROPERTIES FOLDER "qtc_runnable")
endif()
