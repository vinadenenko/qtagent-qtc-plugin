# QLP

# QtAgent — Multi-Provider AI Assistant for Qt Creator

**Repository:** `qtagent-qtc-plugin`  
**Product Name:** QtAgent  

---

## Overview

**QtAgent** is a Qt Creator plugin that brings **AI-powered code assistance** directly into your IDE.  
It provides a side-panel chat interface similar to JetBrains AI or Continue.dev, allowing developers to interact with **local or cloud LLMs** directly from Qt Creator.  

With QtAgent, you can:

- Query AI models about code, design patterns, or logic.  
- Receive **context-aware suggestions** using your current editor content.  
- Quickly **copy, insert, or replace** code snippets from AI responses.  
- [In progress] Manage multiple LLM providers (Ollama, Open WebUI, Claude, Gemini, and more in the future).  

---

## Features

- **Provider-Agnostic Architecture:** Easily switch between LLM backends.  
- **Familiar UI:** Right-hand dock panel with messages, typing indicator, and action buttons.  
- **Async Communication:** Non-blocking requests to keep your IDE responsive.  
- **Settings Integration:** Configure provider URLs, models, and preferences via Qt Creator Options.  
- **Future-Ready:** Designed to add streaming responses, conversation history, and multiple provider support.  

---

## Usage

- Open the **QtAgent** dock panel from the right-hand side.
- Type your query in the input box and press **Send**.
- AI responses appear as bubbles with action buttons:

  - **Copy** → copy to clipboard
  - **Insert** → insert at cursor position
  - **Replace** → replace current selection

## Configuration

Go to **Tools → Options → LLM Provider** to configure:

- **Base URL** – endpoint for your LLM server  
  (example: `http://localhost:11434`)
- **Model** – select the AI model to use  
  (example: `llama3`)

Settings are saved and persist across sessions.

### Planned improvements include:

- Streaming token support
- Multi-provider selection
- Conversation memory and system prompts
- Editor-aware suggestions and code fixes
---

## How to Build

Create a build directory and run

    cmake -DCMAKE_PREFIX_PATH=<path_to_qtcreator> -DCMAKE_BUILD_TYPE=RelWithDebInfo <path_to_plugin_source>
    cmake --build .

where `<path_to_qtcreator>` is the relative or absolute path to a Qt Creator build directory, or to a
combined binary and development package (Windows / Linux), or to the `Qt Creator.app/Contents/Resources/`
directory of a combined binary and development package (macOS), and `<path_to_plugin_source>` is the
relative or absolute path to this plugin directory.

## How to Run

From the command line run

    cmake --build . --target RunQtCreator

`RunQtCreator` is a custom CMake target that will use the <path to qtcreator> referenced above to
start the Qt Creator executable with the following parameters

    -pluginpath <path_to_plugin>

where `<path_to_plugin>` is the path to the resulting plugin library in the build directory
(`<plugin_build>/lib/qtcreator/plugins` on Windows and Linux,
`<plugin_build>/Qt Creator.app/Contents/PlugIns` on macOS).

You might want to add `-temporarycleansettings` (or `-tcs`) to ensure that the opened Qt Creator
instance cannot mess with your user-global Qt Creator settings.
