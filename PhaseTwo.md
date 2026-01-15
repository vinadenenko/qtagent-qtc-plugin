# Phase Two: Roadmap to Production-Ready AI Assistant

This document outlines the strategic plan to elevate this plugin to an enterprise-grade, production-ready AI assistant, rivaling tools like Junie, JetBrains AI, Cursor, and Claude Code.

## 1. Advanced Context & Intelligence (The "Brain")

### 1.1 Project-Wide Semantic Search (RAG)
*   **Vector Database:** Integrate a lightweight local vector database (e.g., ChromaDB or a simple FAISS-based C++ implementation) to index the entire codebase.
*   **Embeddings:** Use local embedding models (via Ollama or ONNX) to ensure privacy while allowing the AI to find relevant code snippets across the whole project, not just open files.
*   **Smart Retrieval:** Implement "Retrieval-Augmented Generation" (RAG) to automatically pull in relevant context based on the user's query.

### 1.2 Deep Semantic Awareness
*   **Clang Code Model Integration:** Deepen integration with Qt Creator's Clang-based indexing.
    *   `find_usages`: Allow the AI to see where a function is used before refactoring.
    *   `type_hierarchy`: Understand class relationships.
    *   `diagnostics`: Feed real-time Clang errors/warnings directly into the AI's context.

### 1.3 Automatic Context Optimization
*   **Dynamic Trimming:** Instead of simple character-based trimming, use a proper tokenizer (like `tiktoken`) and prioritize keeping recent messages and high-relevance code blocks.
*   **Context Pinning:** Allow users to "pin" certain files or documentation URLs to the chat context.

---

## 2. Professional UI/UX (The "Interface")

### 2.1 Side-by-Side Diff & Refactoring Workflow
*   **Diff View:** When the AI suggests code changes, open a side-by-side diff editor (native to Qt Creator) instead of a text block.
*   **One-Click Apply/Reject:** Users should be able to accept or discard specific hunks of the AI's suggestion.
*   **Undo/Redo:** Full integration with the IDE's undo stack for all AI-generated edits.

### 2.2 Terminal & Shell Integration
*   **Integrated Terminal Tool:** Allow the AI to execute shell commands (with user permission). This is critical for running builds, tests, or `git` commands.
*   **Output Analysis:** Automatically capture terminal output (errors, test failures) and feed it back to the AI for self-correction.

### 2.3 Inline "Ghost Text" & Copilot-style Suggestions
*   **Predictive Typing:** Implement a non-intrusive "ghost text" overlay for real-time code completion using small, fast local models.
*   **Contextual Actions:** "Fix this error" buttons appearing next to IDE compiler diagnostics.

---

## 3. Tooling & Ecosystem (The "Capabilities")

### 3.1 Expanded MCP Toolset
*   **`search_code` (Grep):** Real integration with the IDE's search engine for fast keyword lookups.
*   **`get_git_status`:** Allow the AI to see current changes, branches, and commit history.
*   **`run_build_target`:** Trigger specific CMake/QMake targets and analyze results.
*   **`web_search`:** A tool for the AI to fetch documentation or look up library errors online (via Brave Search or similar APIs).

### 3.2 Extensible Tool Architecture
*   **External MCP Servers:** Allow users to connect external MCP servers (e.g., Database, GitHub, Slack) to the plugin, making it a central hub for development.

---

## 4. Robustness & Enterprise Features (The "Stability")

### 4.1 Cost & Token Management
*   **Usage Dashboard:** Show total tokens used per session/day and estimated cost (for OpenAI/Claude).
*   **Budgeting:** Set limits to prevent accidental overspending on expensive models.

### 4.2 Privacy & Security
*   **Local-First Policy:** Prioritize local LLMs (Ollama/LM Studio) and ensure that no code is sent to 3rd party providers unless explicitly configured.
*   **Sensitive Data Masking:** Automatically detect and redact API keys, secrets, or PII before sending context to a remote provider.

### 4.3 Resilience
*   **Model Fallback:** Automatically switch to a backup model (e.g., from GPT-4o to a local Llama-3) if the primary provider is down or rate-limited.
*   **Self-Correction Loop:** If a tool call fails, the AI should be able to analyze the error and retry with different parameters.

---

## 5. Implementation Roadmap

### Phase 2.1: Semantic Foundation (Next 4 Weeks)
1.  Implement local vector indexing for the active project.
2.  Add `search_code` and `get_symbol_info` (via Clang) tools.
3.  Implement proper Tiktoken-based token counting.

### Phase 2.2: The Refactoring Workflow (Next 4 Weeks)
1.  Develop the Side-by-Side Diff UI.
2.  Add "Apply All" and "Reject" buttons to tool-generated patches.
3.  Integrate Terminal execution tool.

### Phase 2.3: Real-time Assistance (Long-term)
1.  Implement inline ghost-text suggestions.
2.  Add support for @-mentions of symbols and files in chat.
3.  Add "Explain this code" context menu actions.
