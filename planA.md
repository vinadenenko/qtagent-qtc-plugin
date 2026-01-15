# QtAgent (QLP) Production-Ready Development Plan

This document outlines the roadmap to transform the current QtAgent plugin into a full-featured, production-ready AI assistant for Qt Creator.
The final version of the plugin will be available with a polished user interface, robust LLM provider support, and seamless integration with Qt Creator's IDE features.
The final version of the plugin should work like continue.dev, JetBrains AI assistant, JetBrains Junie, codex or claude code.

## 1. Core Architecture Enhancements

### 1.1 Robust LLM Provider System
*   **Streaming Support:** Refactor `LLMProvider` and `LLMManager` to support token streaming. This is critical for perceived performance.
*   **Multi-Provider Integration:** Implement concrete providers for:
    *   **OpenAI/Compatible:** Generic OpenAI API support (Claude, DeepSeek, etc., via proxies or direct).
    *   **Google Gemini:** Native integration using Vertex AI or AI Studio APIs. [SKIP]
    *   **Anthropic Claude:** Native integration for Claude 3.5+ models.
*   **Unified Message Format:** Standardize on a format (e.g., OpenAI-like) internally to handle system prompts, user messages, and assistant tool calls consistently.

### 1.2 Conversation Management
*   **History & Context:** Implement a `ConversationHistory` class to maintain state across messages.
*   **Token Counting & Trimming:** Implement context window management to prevent overflowing LLM limits. Show current usage and limits in a status bar.
*   **Persistence:** Save conversations to disk so they can be restored when reopening Qt Creator. [SKIP]

---

## 2. IDE Integration & Tooling (MCP)

### 2.1 Model Context Protocol (MCP) Implementation
*   **MCP Client/Server:** Fully implement the MCP standard to allow the LLM to call tools.
*   **Standard Toolset:**
    *   `read_file`, `write_file`, `create_file`, `delete_file`: Basic FS operations.
    *   `list_directory`: Navigate the project structure.
    *   `search_code`: Integration with Qt Creator's search/grep functionality.
    *   `get_symbol_info`: Use Qt Creator's C++ indexing (ClangCodeModel) to provide deep semantic context.
    *   `run_build`/`run_tests`: Allow the AI to trigger builds and analyze errors. [SKIP]

### 2.2 Advanced Context Injection
*   **Automatic Context:** Automatically include the current open file, selected code, and project tree overview in the system prompt.
*   **Reference Management:** Allow users to @-mention files or symbols to explicitly add them to context.

---

## 3. UI/UX Modernization

### 3.1 Enhanced Chat Interface
*   **Markdown Rendering:** Use a proper Markdown renderer (e.g., based on `QTextDocument` or a light library) for code blocks with syntax highlighting.
*   **Streaming UI:** Update `ChatMessageWidget` to update in real-time as tokens arrive.
*   **Tool Call Visibility:** Show when the AI is "thinking" or "calling a tool" with expandable logs (for example, like Junie does like 'file was read and it's content revealed, file was edited with adding X functionalities etc).

### 3.2 Interaction Workflow
*   **Diff View:** Instead of just "Insert/Replace", show a side-by-side diff (opens with button like it Junie assistant).
*   **Have a button to 'revert' changes made with assistant.
*   **Multi-file Patching:** Support applying changes that span multiple files in one go [SKIP].
*   **Inline Edits (Ghost Text):** (Long term) Implement inline suggestions similar to Copilot [SKIP].

---

## 4. Stability & Production Readiness

### 4.1 Error Handling & Resilience
*   **Retry Logic:** Exponential backoff for network failures.
*   **Graceful Degradation:** Handle cases where a model doesn't support specific tools or has reached rate limits.
*   **Logging:** Implement a dedicated log view for debugging LLM interactions. For example, to be able to show tokens spent in total, in specific prompt, and in specific tool calls (showing tokens spent in each step of the interaction). Get model's capabilities, parameters and limits. Show it to user

### 4.2 Security & Privacy
*   **Credential Storage:** Use a secure store (like Qt's `QSettings` or system keychain) for API keys.
*   **Privacy Controls:** Allow users to blacklist certain files/folders from being sent to the LLM.

### 4.3 Performance
*   **Background Processing:** Ensure all heavy indexing or file operations happen in background threads to never freeze the IDE UI.
*   **Resource Management:** Monitor memory usage of conversation histories.

---

## 5. Development Phases

1.  **Phase 1: Foundation (Weeks 1-2)**
    *   Streaming support in `LLMManager`.
    *   Refined `LLMSettings` and Provider Factory.
    *   Basic Markdown rendering.
2.  **Phase 2: Context & Tools (Weeks 3-5)**
    *   Full `CodeEditorManager` integration (Read/Write/Search).
    *   MCP Tool-call loop implementation.
    *   C++ Indexer integration for "Go to Definition" context.
3.  **Phase 3: UX & Workflow (Weeks 6-8)**
    *   Diff preview for changes.
    *   Buttons like 'apply, reject' etc.
    *   Be able to @-mention files/symbols in the system prompt.
    *   Buttons to interact with the AI (e.g., 'run tool', 'stop tool', 'retry tool', 'view tool logs').
    *   Conversation persistence.
    *   UI polish (animations, better icons).
4.  **Phase 4: Optimization & Release (Weeks 9+)**
    *   Stress testing with large projects.
    *   Documentation and setup guides.
    *   Final binary packaging for Qt Creator versions.
