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
  find_package(Qt6 REQUIRED COMPONENTS Test HttpServer)

  # Tell CMake functions like add_qtc_plugin about the QtTest component.
  set(IMPLICIT_DEPENDS Qt::Test Qt::HttpServer)

  # Enable ctest for auto tests.
  enable_testing()

  add_executable(tst_conversationhistory tests/tst_conversationhistory.cpp)
  target_link_libraries(tst_conversationhistory PRIVATE Qt6::Test Qt6::Core)
  target_include_directories(tst_conversationhistory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(tst_mcpserver 
    tests/tst_mcpserver.cpp 
    src/mcp/mcpserver.cpp
    src/core/codeeditormanager.cpp
  )
  target_link_libraries(tst_mcpserver PRIVATE
    Qt6::Core
    Qt6::Test 
    # Qt6::Widgets
    # Qt6::Network
    QtCreator::Core
    QtCreator::TextEditor
    QtCreator::ProjectExplorer
    # QtCreator::Utils
    # QtCreator::Aggregation
  )
  target_include_directories(tst_mcpserver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(tst_openaiprovider
    tests/tst_openaiprovider.cpp
    src/providers/openai/openaiprovider.cpp
    src/providers/base/llmprovider.cpp
  )
  target_link_libraries(tst_openaiprovider PRIVATE
    Qt6::Test
    Qt6::HttpServer
    Qt6::Network
  )
  target_include_directories(tst_openaiprovider PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(tst_llmmanager
    tests/tst_llmmanager.cpp
    src/llmmanager.cpp
    src/mcp/mcpserver.cpp
    src/core/codeeditormanager.cpp
    src/providers/base/llmprovider.cpp
  )
  target_link_libraries(tst_llmmanager PRIVATE
    Qt6::Test
    Qt6::Widgets
    Qt6::Network
    QtCreator::Core
    QtCreator::TextEditor
    QtCreator::ProjectExplorer
    QtCreator::Utils
    QtCreator::Aggregation
  )
  target_include_directories(tst_llmmanager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(tst_lmstudio_integration
    tests/integration_tests/tst_lmstudio_integration.cpp
    src/providers/openai/openaiprovider.cpp
    src/providers/base/llmprovider.cpp
  )
  target_link_libraries(tst_lmstudio_integration PRIVATE
    Qt6::Test
    Qt6::Network
  )
  target_include_directories(tst_lmstudio_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(tst_tooling_integration
    tests/integration_tests/tst_tooling_integration.cpp
    src/llmmanager.cpp
    src/providers/openai/openaiprovider.cpp
    src/providers/base/llmprovider.cpp
    src/mcp/mcpserver.cpp
    src/core/codeeditormanager.cpp
  )
  target_link_libraries(tst_tooling_integration PRIVATE
    Qt6::Test
    Qt6::Network
    Qt6::Widgets
    QtCreator::Core
    QtCreator::TextEditor
    QtCreator::ProjectExplorer
  )
  target_include_directories(tst_tooling_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_qtc_plugin(QLP
  PLUGIN_DEPENDS
    QtCreator::Core
    QtCreator::TextEditor
    QtCreator::ProjectExplorer
  DEPENDS
    Qt::Widgets
    Qt::Network
    QtCreator::ExtensionSystem
    QtCreator::Utils
    QtCreator::Aggregation
  SOURCES
    # .github/workflows/README.md
    # .github/workflows/build_cmake.yml
    README.md
    qlp.cpp
    qlpconstants.h
    qlptr.h
)

target_sources(QLP
  PRIVATE
    src/ui/chatdockwidget.h src/ui/chatdockwidget.cpp
    src/ui/chatmessagewidget.h src/ui/chatmessagewidget.cpp
    src/ui/chatfooterwidget.h src/ui/chatfooterwidget.cpp
    src/ui/llmoptionspage.h src/ui/llmoptionspage.cpp
    src/ui/typingindicatorwidget.h src/ui/typingindicatorwidget.cpp

    src/providers/base/llmprovider.h src/providers/base/llmprovider.cpp
    src/providers/ollama/ollamaprovider.h src/providers/ollama/ollamaprovider.cpp
    src/providers/openai/openaiprovider.h src/providers/openai/openaiprovider.cpp
    src/providers/claude/claudeprovider.h src/providers/claude/claudeprovider.cpp

    src/llmmanager.h src/llmmanager.cpp
    src/core/conversationhistory.h

    src/settings/llmsettings.h src/settings/llmsettings.cpp
    src/core/codeeditormanager.h src/core/codeeditormanager.cpp
    src/mcp/mcpserver.h src/mcp/mcpserver.cpp
)

# Enable the Run button in Qt Creator
get_target_property(QtCreatorCorePath QtCreator::Core LOCATION)
message("QtCreatorCore lib path: " ${QtCreatorCorePath})
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
