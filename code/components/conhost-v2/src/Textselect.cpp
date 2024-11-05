// Copyright 2024 Aidan Sun and the ImGuiTextSelect contributors
// SPDX-License-Identifier: MIT

#include "StdInc.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <string_view>

#define IMGUI_DEFINE_MATH_OPERATORS
#define GImGui ImGui::GetCurrentContext()
#include <imgui.h>
#include <imgui_internal.h>
#include <utf8.h>

#include <range/v3/algorithm/find_if.hpp>

#include "Textselect.hpp"

// Simple word boundary detection, accounts for Latin Unicode blocks only.
static bool isBoundary(char32_t c) {
	using Range = std::array<char32_t, 2>;
	std::array<Range, 4> ranges{
		Range{ 0x20, 0x2F },
		Range{ 0x3A, 0x40 },
		Range{ 0x5B, 0x60 },
		Range{ 0x7B, 0xBF }
	};

	return ranges::find_if(ranges, [c](const Range& r) { return c >= r[0] && c <= r[1]; }) != ranges.end();
}

// Gets the number of UTF-8 characters (not bytes) in a string.
static std::size_t utf8Length(std::string_view s) {
    return utf8::unchecked::distance(s.begin(), s.end());
}

// Gets the display width of a substring.
static float substringSizeX(std::string_view s, const std::size_t start, const std::size_t length = std::string_view::npos) {
    // Convert char-based start and length into byte-based iterators
    auto stringStart = s.begin();
    utf8::unchecked::advance(stringStart, start);

    auto stringEnd = stringStart;
    if (length == std::string_view::npos) stringEnd = s.end();
    else utf8::unchecked::advance(stringEnd, std::min(utf8Length(s), length));

    // Dereferencing std::string_view::end() may be undefined behavior in some compilers,
    // because of that, we need to get the pointer value manually if stringEnd == s.end().
    const char* endPtr = stringEnd == s.end() ? s.data() + s.size() : &*stringEnd;

    // Calculate text size between start and end
    return ImGui::CalcTextSize(&*stringStart, endPtr).x;
}

// Gets the index of the character the mouse cursor is over.
static std::size_t getCharIndex(const std::string_view s, const float cursorPosX, const std::size_t start, const std::size_t end) {
    // Ignore cursor position when it is invalid
    if (cursorPosX < 0) return 0;

    // Check for exit conditions
    if (s.empty()) return 0;
    if (end < start) return utf8Length(s);

    // Midpoint of given string range
	const std::size_t midIdx = start + (end - start) / 2;

    // Display width of the entire string up to the midpoint, gives the x-position where the (midIdx + 1)th char starts
    const float widthToMid = substringSizeX(s, 0, midIdx + 1);

    // Same as above but exclusive, gives the x-position where the (midIdx)th char starts

    // Perform a recursive binary search to find the correct index
    // If the mouse position is between the (midIdx)th and (midIdx + 1)th character positions, the search ends
    if (cursorPosX < substringSizeX(s, 0, midIdx)) return getCharIndex(s, cursorPosX, start, midIdx - 1);
    if (cursorPosX > widthToMid) return getCharIndex(s, cursorPosX, midIdx + 1, end);
    return midIdx;
}

// Wrapper for getCharIndex providing the initial bounds.
static std::size_t getCharIndex(const std::string_view s, const float cursorPosX) {
    return getCharIndex(s, cursorPosX, 0, utf8Length(s));
}

// Gets the scroll delta for the given cursor position and window bounds.
static float getScrollDelta(const float v, const float min, const float max) {
    const float deltaScale = 10.0f * ImGui::GetIO().DeltaTime;
    constexpr float maxDelta = 100.0f;

    if (v < min) return std::max(-(min - v), -maxDelta) * deltaScale;
    else if (v > max) return std::min(v - max, maxDelta) * deltaScale;

    return 0.0f;
}

TextSelect::Selection TextSelect::getSelection() {
    // Start and end may be out of order (ordering is based on Y position)
    const bool startBeforeEnd = selectStart.y < selectEnd.y || (selectStart.y == selectEnd.y && selectStart.x < selectEnd.x);

    // Reorder X points if necessary
    const std::size_t startX = startBeforeEnd ? selectStart.x : selectEnd.x;
    const std::size_t endX = startBeforeEnd ? selectEnd.x : selectStart.x;

    // Get min and max Y positions for start and end
    const std::size_t startY = std::min(selectStart.y, selectEnd.y);
    const std::size_t endY = std::max(selectStart.y, selectEnd.y);

    if (const std::size_t numLines = getNumLines(); startY >= numLines || endY >= numLines)
    {
        selectStart = { std::string_view::npos, std::string_view::npos };
        selectEnd = { std::string_view::npos, std::string_view::npos };
        return { 0, 0, 0, 0 };
    }

    return { startX, startY, endX, endY };
}

void TextSelect::handleMouseDown(const ImVec2& cursorPosStart) {
    const float textHeight = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2 mousePos = ImGui::GetMousePos() - cursorPosStart;

    // Get Y position of mouse cursor, in terms of line number (capped to the index of the last line)
    const std::size_t y = std::min(static_cast<std::size_t>(std::floor(mousePos.y / textHeight)), getNumLines() - 1);

    std::string_view currentLine = getLineAtIdx(y);
    const float offset = getTextOffset(y);
    const std::size_t x = getCharIndex(currentLine, mousePos.x - offset);

    // Get mouse click count and determine action
    if (const int mouseClicks = ImGui::GetMouseClickedCount(ImGuiMouseButton_Left); mouseClicks > 0) {
        const ImGuiIO& io = ImGui::GetIO();
        if (mouseClicks % 3 == 0) {
            // Triple click - select line
            selectStart = { 0, y };
            selectEnd = { utf8Length(currentLine), y };
        } else if (mouseClicks % 2 == 0) {
            // Double click - select word
            // Initialize start and end iterators to current cursor position
            utf8::unchecked::iterator startIt{ currentLine.data() };
            utf8::unchecked::iterator endIt{ currentLine.data() };
            for (std::size_t i = 0; i < x; i++) {
                ++startIt;
                ++endIt;
            }

            bool isCurrentBoundary = isBoundary(*startIt);

            // Scan to left until a word boundary is reached
            for (std::size_t startInv = 0; startInv <= x; startInv++) {
                if (isBoundary(*startIt) != isCurrentBoundary) break;
                selectStart = { x - startInv, y };
                --startIt;
            }

            // Scan to right until a word boundary is reached
            for (std::size_t end = x; end <= utf8Length(currentLine); end++) {
                selectEnd = { end, y };
                if (isBoundary(*endIt) != isCurrentBoundary) break;
                ++endIt;
            }
        } else if (io.KeyShift) {
            // Single click with shift - select text from start to click
            // The selection starts from the beginning if no start position exists
            if (selectStart.isInvalid()) selectStart = { 0, 0 };

            selectEnd = { x, y };
        } else {
            // Single click - set start position, invalidate end position
            selectStart = { x, y };
            selectEnd = { std::string_view::npos, std::string_view::npos };
        }
    } else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        // Mouse dragging - set end position
        selectEnd = { x, y };
    }
}

void TextSelect::handleScrolling() {
    // Window boundaries
    const ImVec2 windowMin = ImGui::GetWindowPos();
    const ImVec2 windowMax = windowMin + ImGui::GetWindowSize();

    // Get current and active window information from Dear ImGui state
    ImGuiWindow* currentWindow = ImGui::GetCurrentWindow();
    const ImGuiWindow* activeWindow = GImGui->ActiveIdWindow;

    const ImGuiID scrollXID = ImGui::GetWindowScrollbarID(currentWindow, ImGuiAxis_X);
    const ImGuiID scrollYID = ImGui::GetWindowScrollbarID(currentWindow, ImGuiAxis_Y);
    const ImGuiID activeID = ImGui::GetActiveID();
    const bool scrollbarsActive = activeID == scrollXID || activeID == scrollYID;

    // Do not handle scrolling if:
    // - There is no active window
    // - The current window is not active
    // - The user is scrolling via the scrollbars
    if (activeWindow == nullptr || activeWindow->ID != currentWindow->ID || scrollbarsActive) return;

    // Get scroll deltas from mouse position
    const ImVec2 mousePos = ImGui::GetMousePos();
    const float scrollXDelta = getScrollDelta(mousePos.x, windowMin.x, windowMax.x);
    const float scrollYDelta = getScrollDelta(mousePos.y, windowMin.y, windowMax.y);

    // If there is a nonzero delta, scroll in that direction
    if (std::abs(scrollXDelta) > 0.0f) ImGui::SetScrollX(ImGui::GetScrollX() + scrollXDelta);
    if (std::abs(scrollYDelta) > 0.0f) ImGui::SetScrollY(ImGui::GetScrollY() + scrollYDelta);
}

void TextSelect::drawSelection(const ImVec2& cursorPosStart) {
    if (!hasSelection()) return;

    // Start and end positions
    auto [startX, startY, endX, endY] = getSelection();

    // Add a rectangle to the draw list for each line contained in the selection
    for (std::size_t i = startY; i <= endY; i++) {
        const std::string_view line = getLineAtIdx(i);
        const float offset = getTextOffset(i);

        // Display sizes
        // The width of the space character is used for the width of newlines.
        const float newlineWidth = ImGui::CalcTextSize(" ").x;
        const float textHeight = ImGui::GetTextLineHeightWithSpacing();

        // The first and last rectangles should only extend to the selection boundaries
        // The middle rectangles (if any) enclose the entire line + some extra width for the newline.
        const float minX = (i == startY ? substringSizeX(line, 0, startX) : 0) + offset;
        const float maxX = (i == endY ? substringSizeX(line, 0, endX) : substringSizeX(line, 0) + newlineWidth) + offset;

        // Rectangle height equals text height
        const float minY = static_cast<float>(i) * textHeight;
        const float maxY = static_cast<float>(i + 1) * textHeight;

        // Get rectangle corner points offset from the cursor's start position in the window
        ImVec2 rectMin = cursorPosStart + ImVec2{ minX, minY };
        ImVec2 rectMax = cursorPosStart + ImVec2{ maxX, maxY };

        // Draw the rectangle
        const ImU32 color = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
        ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, color);
    }
}

void TextSelect::copy() {
    if (!hasSelection()) return;

    auto [startX, startY, endX, endY] = getSelection();

    // Collect selected text in a single string
    std::string selectedText;

    for (std::size_t i = startY; i <= endY; i++) {
        // Similar logic to drawing selections
        const std::size_t subStart = i == startY ? startX : 0;
        std::string_view line = getLineAtIdx(i);

        auto stringStart = line.begin();
        utf8::unchecked::advance(stringStart, subStart);

        auto stringEnd = stringStart;
        if (i == endY) utf8::unchecked::advance(stringEnd, endX - subStart);
        else stringEnd = line.end();

        std::string_view lineToAdd = line.substr(stringStart - line.begin(), stringEnd - stringStart);
        selectedText += lineToAdd;

        // If lines before the last line don't already end with newlines, add them in
		if (!(lineToAdd.empty() && lineToAdd.back() == '\n') && i < endY) selectedText += '\n';

    }

	if (!selectedText.empty())
	{
        ImGui::SetClipboardText(selectedText.c_str());
	}
}

void TextSelect::selectAll() {
    const std::size_t lastLineIdx = getNumLines() - 1;
    const std::string_view lastLine = getLineAtIdx(lastLineIdx);

    // Set the selection range from the beginning to the end of the last line
    selectStart = { 0, 0 };
    selectEnd = { utf8Length(lastLine), lastLineIdx };
}

void TextSelect::update(const std::size_t itemOffset) {
    if (itemOffset > 0)
    {
		if (selectStart.y >= itemOffset)
		{
			selectStart.y -= itemOffset;
		}
        else
        {
            selectStart.y = 0;
        }

        if (selectEnd.y >= itemOffset)
        {
            selectEnd.y -= itemOffset;
        }
        else
        {
            selectStart = { std::string_view::npos, std::string_view::npos };
            selectEnd = { std::string_view::npos, std::string_view::npos };
        }
    }

    // ImGui::GetCursorStartPos() is in window coordinates so it is added to the window position
    const ImVec2 cursorPosStart = ImGui::GetWindowPos() + ImGui::GetCursorStartPos();

    // Switch cursors if the window is hovered
    const bool hovered = ImGui::IsWindowHovered();
    if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

    // Handle mouse events
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (hovered) handleMouseDown(cursorPosStart);
        else handleScrolling();
    }

    drawSelection(cursorPosStart);

    const ImGuiIO& io = ImGui::GetIO();

    // Keyboard shortcuts
    if (ImGui::IsWindowFocused()) {
        ImGui::CaptureKeyboardFromApp(true);
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
            selectAll();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
            copy();
        }
	}
}
