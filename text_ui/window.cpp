#include "window.h"

#include <limits>

namespace terminal_editor {

Window::Window(WindowManager* windowManager, const std::string& name, Rect rect)
    : m_windowManager(windowManager)
    , m_name(name)
    , m_parent(nullptr)
    , m_rect(rect)
{
    m_windowManager->windowCreated(this);
}

Window::~Window() {
    m_windowManager->windowDestroyed(this);
}

void Window::close() {
    ZASSERT(m_parent) << "Cannot close window that doesn't have a parent.";
    auto saveParent = m_parent;
    auto self = m_parent->removeChild(this);
    auto focus = m_windowManager->getFocusedWindow();
    if (focus == this) {
        m_windowManager->setFocusedWindow(saveParent);
    }
}

bool Window::doProcessAction(const std::string& action) {
    if (action == "close") {
        if (m_parent) {
            close();
        }
        return true;
    }

    if (action == "hello") {
        messageBox(this, "Hello!");
        return true;
    }

    return false;
}

void BasicWindow::drawSelf(ScreenCanvas& windowCanvas) {
    auto localRect = Rect{Point{0, 0}, getRect().size};
    auto attributes = m_attributes;
    if (getWindowManager()->getFocusedWindow() == this) {
        attributes.fgColor = Color::Bright_Red;
    }
    windowCanvas.fillRect(localRect, m_doubleEdge, true, attributes);

    auto messageLength = getRenderedWidth(m_message);

    auto point = localRect.center();
    point.x = (localRect.size.width - messageLength) / 2 - 1;
    point.y--;
    auto textCanvas = windowCanvas.getSubCanvas({{1, 1}, Size{localRect.size.width - 2, localRect.size.height - 2}});
    textCanvas.print(point, m_message, m_attributes, m_attributes, m_attributes);
}

bool BasicWindow::doProcessAction(const std::string& action) {
    if (action == "left") {
        getRect().move({-1, 0});
        return true;
    }

    if (action == "right") {
        getRect().move({1, 0});
        return true;
    }

    if (action == "up") {
        getRect().move({0, -1});
        return true;
    }

    if (action == "down") {
        getRect().move({0, 1});
        return true;
    }

    if (action == "size-left") {
        getRect().size.width -= 1;
        return true;
    }

    if (action == "size-right") {
        getRect().size.width += 1;
        return true;
    }

    if (action == "size-up") {
        getRect().size.height -= 1;
        return true;
    }

    if (action == "size-down") {
        getRect().size.height += 1;
        return true;
    }

    return Window::doProcessAction(action);
}

Window* messageBox(Window* parent, const std::string& message) {
    auto width = getRenderedWidth(message) + 4;
    auto height = 3;

    auto rect = parent->getScreenRect();
    auto relX = (rect.size.width - width) / 2;
    auto relY = (rect.size.height - height) / 2;

    rect.move(Size{relX, relY});
    rect.size = Size{width, height};

    auto boxWindow = parent->addChild<BasicWindow>("Message Box", rect, true, Attributes{Color::White, Color::Green, Style::Normal});
    boxWindow->setMessage(message);
    parent->getWindowManager()->setFocusedWindow(boxWindow);
    return boxWindow;
};

} // namespace terminal_editor
