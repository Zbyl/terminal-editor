#include "text_buffer.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace terminal_editor {

std::vector<std::string> splitString(const std::string& text, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;

    std::istringstream tokenStream(text);
    bool lastLineHadEof = true;
    while (std::getline(tokenStream, token, delimiter))
    {
        lastLineHadEof = tokenStream.eof();
        tokens.push_back(token);
    }

    if (!lastLineHadEof) {
        tokens.push_back({});
    }

    return tokens;
}

void TextBuffer::loadFile(const std::string& fileName) {
    std::ifstream input;
    input.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    input.open(fileName, std::ios::binary);

    std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>()); // @todo Super inefficient.

    lines = splitString(text, '\n');
}

int TextBuffer::getNumberOfLines() const {
    return static_cast<int>(lines.size());
}

int TextBuffer::getLongestLineLength() const {
    // @todo This is a stub implementation.
    auto longestPos = std::max_element(lines.begin(), lines.end(), [](const auto& text0, const auto& text1) { return text0.size() < text1.size(); } );
    if (longestPos == lines.end())
        return 0;
    return static_cast<int>(longestPos->size());
}

std::string TextBuffer::getLine(int row) const {
    if (row < 0)
        return {};

    if (row >= static_cast<int>(lines.size()))
        return {};

    return lines[row];
}

std::string TextBuffer::getLineRange(int row, int colStart, int colEnd) const {
    auto line = getLine(row);

    auto lineLength = static_cast<int>(line.size());

    colStart = std::max(colStart, 0);
    colStart = std::min(colStart, lineLength);

    colEnd = std::max(colEnd, 0);
    colEnd = std::min(colEnd, lineLength);

    return line.substr(colStart, colEnd - colStart);
}

Position TextBuffer::insertText(Position position, const std::string& text) {
    position = clampPosition(position);
    bool linePastEnd = position.row > static_cast<int>(lines.size());

    auto lineParts = splitString(text, '\n');

    auto line = getLine(position.row);
    if (lineParts.size() == 1) {
        auto modifiedLine = line.insert(position.column, lineParts.front());
        if (linePastEnd) {
            lines.push_back(modifiedLine);
        } else {
            lines[position.row] = modifiedLine;
        }

        auto endPosition = position;
        endPosition.column += static_cast<int>(lineParts.front().size());
        return endPosition;
    }

    // Current line should be split,
    // then front should have first part appended,
    // back should have last part appended,
    // all other parts should be put in-between.

    auto endPosition = position;
    endPosition.row += static_cast<int>(lineParts.size()) - 1;
    endPosition.column = static_cast<int>(lineParts.back().size());

    auto lineFront = line.substr(0, position.column);
    auto lineBack = line.substr(position.column);
    lineParts.front() = lineFront + lineParts.front();
    lineParts.back() += lineBack;

    // Replace line with lineParts.
    if (!linePastEnd)
        lines.erase(lines.begin() + position.row);

    lines.insert(lines.begin() + position.row, lineParts.begin(), lineParts.end());

    return endPosition;
}

Position TextBuffer::clampPosition(Position position) {
    position.row = std::max(position.row, 0);
    position.row = std::min(position.row, getNumberOfLines());

    auto lineLength = lines.empty() ? 0 : static_cast<int>(lines[position.row].size());

    position.column = std::max(position.column, 0);
    position.column = std::min(position.column, lineLength);

    return position;
}

} // namespace terminal_editor
