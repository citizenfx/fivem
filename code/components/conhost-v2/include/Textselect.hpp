// Copyright 2024 Aidan Sun and the ImGuiTextSelect contributors
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <functional>

#include <imgui.h>

// Manages text selection in a GUI window.
// This class only works if the window only has text, and line wrapping is not supported.
// The window should also have the "NoMove" flag set so mouse drags can be used to select text.
class TextSelect {
    // Cursor position in the window.
    struct CursorPos {
        std::size_t x = std::string_view::npos; // X index of character
        std::size_t y = std::string_view::npos; // Y index of character

        // Checks if this position is invalid.
        bool isInvalid() const {
            // Invalid cursor positions are indicated by std::string::npos
            return x == std::string_view::npos || y == std::string_view::npos;
        }
    };

    // Text selection in the window.
    struct Selection {
        std::size_t startX;
        std::size_t startY;
        std::size_t endX;
        std::size_t endY;
    };

    // Selection bounds
    // In a selection, the start and end positions may not be in order (the user can click and drag left/up which
    // reverses start and end).
    CursorPos selectStart;
    CursorPos selectEnd;

    // Accessor functions to get line information
    // This class only knows about line numbers so it must be provided with functions that give it text data.
    std::function<std::string_view(std::size_t)> getLineAtIdx; // Gets the string given a line number
    std::function<std::size_t()> getNumLines; // Gets the total number of lines
    std::function<float(std::size_t)> getTextOffset; // Gets the offset of the text

    // Gets the user selection. Start and end are guaranteed to be in order.
    Selection getSelection();

    // Processes mouse down (click/drag) events.
    void handleMouseDown(const ImVec2& cursorPosStart);

    // Processes scrolling events.
    static void handleScrolling();

    // Draws the text selection rectangle in the window.
    void drawSelection(const ImVec2& cursorPosStart);

public:
    // Sets the text accessor functions.
    // getLineAtIdx: Function taking a std::size_t (line number) and returning the string in that line
    // getNumLines: Function returning a std::size_t (total number of lines of text)
    // getTextOffset: Function taking a std::size_t (line number) and returning the offset of the text as a float
    template <class T, class U, class V>
    TextSelect(const T& getLineAtIdx, const U& getNumLines, const V& getTextOffset) : getLineAtIdx(getLineAtIdx), getNumLines(getNumLines), getTextOffset(getTextOffset) {}

    template <class T, class U>
    TextSelect(const T& getLineAtIdx, const U& getNumLines) : TextSelect(getLineAtIdx, getNumLines, []() { return 0.0f; }) {}

    // Checks if there is an active selection in the text.
    bool hasSelection() const {
        return !selectStart.isInvalid() && !selectEnd.isInvalid();
    }

    // Copies the selected text to the clipboard.
    void copy();

    // Selects all text in the window.
    void selectAll();

    // Draws the text selection rectangle and handles user input.
    void update(std::size_t itemOffset = 0);
};
