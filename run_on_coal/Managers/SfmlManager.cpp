#include "stdafx.h"

#include "Managers/SfmlManager.h"
#include "Core/Core.h"
#include "Lua/LuaArguments.h"

#include "Managers/ConfigManager.h"
#include "Managers/EventManager.h"
#include "Managers/LogManager.h"
#include "Managers/LuaManager.h"
#include "Managers/PhysicsManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Elements/Shader/Shader.h"

#define ROC_OPENGL_MIN_VERSION 31U
#define ROC_OPENGL_MIN_VERSION_STRING "3.1"

namespace ROC
{

extern const std::vector<std::string> g_KeyNamesTable;
extern const std::vector<std::string> g_MouseKeyNamesTable;
extern const std::vector<std::string> g_JoypadAxisNamesTable;

}

ROC::SfmlManager::SfmlManager(Core *f_core)
{
    m_core = f_core;

    m_time = 0.f;

    std::string l_log;

    ConfigManager *l_configManager = m_core->GetConfigManager();
    const glm::ivec2 &l_windowSize = l_configManager->GetWindowSize();
    m_windowVideoMode = sf::VideoMode(l_windowSize.x, l_windowSize.y);

    m_contextSettings.antialiasingLevel = static_cast<unsigned int>(l_configManager->GetAntialiasing());
    m_contextSettings.depthBits = 32U;
#ifdef _DEBUG
    m_contextSettings.attributeFlags = sf::ContextSettings::Attribute::Debug;
#else
    m_contextSettings.attributeFlags = sf::ContextSettings::Attribute::Default;
#endif
    m_contextSettings.majorVersion = 0U;
    m_contextSettings.minorVersion = 0U;
    m_contextSettings.sRgbCapable = false;
    m_contextSettings.stencilBits = 0U;

    m_windowStyle = (l_configManager->IsFullscreenEnabled() ? sf::Style::Fullscreen : sf::Style::Default);

    m_window = new sf::Window(m_windowVideoMode, "RunOnCoal Engine Application", m_windowStyle, m_contextSettings);
    m_window->setActive(true);
    m_active = true;

    if(glGetString(GL_VERSION) == NULL)
    {
        l_log.assign("SFML: Unable to create OpenGL ");
        l_log.append(std::to_string(m_contextSettings.majorVersion));
        l_log.push_back('.');
        l_log.append(std::to_string(m_contextSettings.minorVersion));
        l_log.append(" context. Check supported version for your videocard.");
        m_core->GetLogManager()->Log(l_log);

        MessageBoxA(m_window->getSystemHandle(), l_log.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
        exit(EXIT_FAILURE);
    }
    else
    {
        sf::ContextSettings l_createdContextSettings = m_window->getSettings();
        if(l_createdContextSettings.majorVersion * 10U + l_createdContextSettings.minorVersion < ROC_OPENGL_MIN_VERSION)
        {
            l_log.assign("SFML: Minimal supported version of OpenGL is ");
            l_log.append(ROC_OPENGL_MIN_VERSION_STRING);
            m_core->GetLogManager()->Log(l_log);

            MessageBoxA(m_window->getSystemHandle(), l_log.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
            exit(EXIT_FAILURE);
        }
    }
    m_frameLimit = l_configManager->GetFPSLimit();
    m_window->setFramerateLimit(m_frameLimit);
    m_window->setVerticalSyncEnabled(l_configManager->GetVSync());
    m_window->setKeyRepeatEnabled(false);

    GLenum l_error = glewInit();
    if(l_error != GLEW_OK)
    {
        l_log.assign("GLEW: ");
        l_log.append(reinterpret_cast<const char*>(glewGetErrorString(l_error)));
        m_core->GetLogManager()->Log(l_log);

        MessageBoxA(m_window->getSystemHandle(), l_log.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
        exit(EXIT_FAILURE);
    }

    l_log.assign("OpenGL ");
    l_log.append(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    l_log.append(", ");
    l_log.append(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    l_log.append(", ");
    l_log.append("GLEW ");
    l_log.append(reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));
    m_core->GetLogManager()->Log(l_log);

    m_inputState = false;

    m_luaArguments = new LuaArguments();

    m_windowResizeCallback = nullptr;
    m_windowFocusCallback = nullptr;
    m_keyPressCallback = nullptr;
    m_textInputCallback = nullptr;
    m_mouseMoveCallback = nullptr;
    m_cursorEnterCallback = nullptr;
    m_mouseKeyPressCallback = nullptr;
    m_mouseScrollCallback = nullptr;
    m_joypadStateChangeCallback = nullptr;
    m_joypadButtonCallback = nullptr;
    m_joypadAxisCallback = nullptr;
}
ROC::SfmlManager::~SfmlManager()
{
    m_window->setActive(false);
    m_window->close();
    delete m_window;
    delete m_luaArguments;
}

void ROC::SfmlManager::GetWindowPosition(glm::ivec2 &f_pos) const
{
    sf::Vector2i l_position = m_window->getPosition();
    f_pos.x = l_position.x;
    f_pos.y = l_position.y;
}
void ROC::SfmlManager::SetWindowPosition(const glm::ivec2 &f_pos)
{
    sf::Vector2i l_position(f_pos.x, f_pos.y);
    m_window->setPosition(l_position);
}
void ROC::SfmlManager::GetWindowSize(glm::ivec2 &f_size) const
{
    sf::Vector2u l_size = m_window->getSize();
    f_size.x = static_cast<int>(l_size.x);
    f_size.y = static_cast<int>(l_size.y);
}
void ROC::SfmlManager::SetFramelimit(unsigned int f_fps)
{
    if(f_fps != m_frameLimit)
    {
        m_frameLimit = f_fps;
        m_window->setFramerateLimit(m_frameLimit);
        m_core->GetPhysicsManager()->UpdateWorldSteps(m_frameLimit);
    }
}
bool ROC::SfmlManager::SetIcon(const std::string &f_path)
{
    bool l_result = false;

    sf::Image l_image;
    if(l_image.loadFromFile(f_path))
    {
        sf::Vector2u l_size = l_image.getSize();
        m_window->setIcon(l_size.x, l_size.y, l_image.getPixelsPtr());
        l_result = true;
    }
    return l_result;
}

void ROC::SfmlManager::SetCursorMode(bool f_visible, bool f_lock)
{
    m_window->setMouseCursorGrabbed(f_lock);
    m_window->setMouseCursorVisible(f_visible);
}
void ROC::SfmlManager::GetCursorPosition(glm::ivec2 &f_pos) const
{
    sf::Vector2i l_position = sf::Mouse::getPosition(*m_window);
    std::memcpy(&f_pos, &l_position, sizeof(glm::ivec2));
}
void ROC::SfmlManager::SetCursorPosition(const glm::ivec2 &f_pos)
{
    sf::Mouse::setPosition((sf::Vector2i&)f_pos, *m_window);
}

void ROC::SfmlManager::GetClipboardString(std::string &f_str) const
{
    sf::String l_string = sf::Clipboard::getString();
    f_str.assign(l_string);
}
void ROC::SfmlManager::SetClipboardString(const std::string &f_str)
{
    sf::Clipboard::setString(f_str);
}

bool ROC::SfmlManager::IsKeyPressed(int f_key)
{
    return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(f_key));
}
bool ROC::SfmlManager::IsMouseKeyPressed(int f_key)
{
    return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(f_key));
}

bool ROC::SfmlManager::IsJoypadConnected(unsigned int f_jp)
{
    return sf::Joystick::isConnected(f_jp);
}
bool ROC::SfmlManager::GetJoypadButtonState(unsigned int f_jp, unsigned int f_button)
{
    return sf::Joystick::isButtonPressed(f_jp, f_button);
}
unsigned int ROC::SfmlManager::GetJoypadButtonCount(unsigned int f_jp)
{
    return sf::Joystick::getButtonCount(f_jp);
}
bool ROC::SfmlManager::CheckJoypadAxis(unsigned int f_jp, unsigned int f_axis)
{
    return sf::Joystick::hasAxis(f_jp, static_cast<sf::Joystick::Axis>(f_axis));
}
float ROC::SfmlManager::GetJoypadAxisValue(unsigned int f_jp, unsigned int f_axis)
{
    return sf::Joystick::getAxisPosition(f_jp, static_cast<sf::Joystick::Axis>(f_axis));
}

bool ROC::SfmlManager::DoPulse()
{
    m_time = m_clock.getElapsedTime().asSeconds();

    bool l_mouseFix = false;
    while(m_window->pollEvent(m_event))
    {
        switch(m_event.type)
        {
            case sf::Event::Closed:
                m_active = false;
                break;
            case sf::Event::Resized:
            {
                glm::ivec2 l_size(static_cast<int>(m_event.size.width), static_cast<int>(m_event.size.height));
                m_core->GetRenderManager()->UpdateViewportSize(l_size);

                if(m_windowResizeCallback) (*m_windowResizeCallback)(m_event.size.width, m_event.size.height);

                m_luaArguments->PushArgument(l_size.x);
                m_luaArguments->PushArgument(l_size.y);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onWindowResize, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::GainedFocus: case sf::Event::LostFocus:
            {
                if(m_windowFocusCallback) (*m_windowFocusCallback)(m_event.type == sf::Event::GainedFocus);

                m_luaArguments->PushArgument(m_event.type == sf::Event::GainedFocus ? 1 : 0);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onWindowFocus, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::KeyPressed: case sf::Event::KeyReleased:
            {
                if(m_event.key.code != -1)
                {
                    if(m_keyPressCallback) (*m_keyPressCallback)(m_event.key.code, m_event.type == sf::Event::KeyPressed);

                    m_luaArguments->PushArgument(g_KeyNamesTable[m_event.key.code]);
                    m_luaArguments->PushArgument(m_event.type == sf::Event::KeyPressed ? 1 : 0);
                    m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onKeyPress, m_luaArguments);
                    m_luaArguments->Clear();
                }
            } break;
            case sf::Event::TextEntered:
            {
                if(m_inputState)
                {
                    if(m_event.text.unicode > 31 && !(m_event.text.unicode >= 127 && m_event.text.unicode <= 160))
                    {
                        sf::String l_text(m_event.text.unicode);
                        std::basic_string<unsigned char> l_utf8 = l_text.toUtf8();
                        std::string l_input(l_utf8.begin(), l_utf8.end());

                        if(m_textInputCallback) (*m_textInputCallback)(l_input);

                        m_luaArguments->PushArgument(l_input);
                        m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onTextInput, m_luaArguments);
                        m_luaArguments->Clear();
                    }
                }
            } break;
            case sf::Event::MouseMoved:
            {
                if(!l_mouseFix) // Prevent loop if cursor position is changed in onCursorMove event
                {
                    if(m_mouseMoveCallback) (*m_mouseMoveCallback)(m_event.mouseMove.x, m_event.mouseMove.y);

                    m_luaArguments->PushArgument(m_event.mouseMove.x);
                    m_luaArguments->PushArgument(m_event.mouseMove.y);
                    m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onCursorMove, m_luaArguments);
                    m_luaArguments->Clear();
                    l_mouseFix = true;
                }
            } break;
            case sf::Event::MouseEntered: case sf::Event::MouseLeft:
            {
                if(m_cursorEnterCallback) (*m_cursorEnterCallback)(m_event.type == sf::Event::MouseEntered);

                m_luaArguments->PushArgument(m_event.type == sf::Event::MouseEntered ? 1 : 0);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onCursorEnter, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::MouseButtonPressed: case sf::Event::MouseButtonReleased:
            {
                if(m_mouseKeyPressCallback) (*m_mouseKeyPressCallback)(m_event.mouseButton.button, m_event.type == sf::Event::MouseButtonPressed);

                m_luaArguments->PushArgument(g_MouseKeyNamesTable[m_event.mouseButton.button]);
                m_luaArguments->PushArgument(m_event.type == sf::Event::MouseButtonPressed ? 1 : 0);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onMouseKeyPress, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::MouseWheelScrolled:
            {
                if(m_mouseScrollCallback) (*m_mouseScrollCallback)(m_event.mouseWheelScroll.wheel, m_event.mouseWheelScroll.delta);

                m_luaArguments->PushArgument(m_event.mouseWheelScroll.wheel);
                m_luaArguments->PushArgument(m_event.mouseWheelScroll.delta);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onMouseScroll, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::JoystickConnected: case sf::Event::JoystickDisconnected:
            {
                if(m_joypadStateChangeCallback) (*m_joypadStateChangeCallback)(m_event.joystickConnect.joystickId, m_event.type == sf::Event::JoystickConnected);

                m_luaArguments->PushArgument(static_cast<int>(m_event.joystickConnect.joystickId));
                m_luaArguments->PushArgument(m_event.type == sf::Event::JoystickConnected ? 1 : 0);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onJoypadStateChange, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::JoystickButtonPressed: case sf::Event::JoystickButtonReleased:
            {
                if(m_joypadButtonCallback) (*m_joypadButtonCallback)(m_event.joystickButton.joystickId, m_event.joystickButton.button, m_event.type == sf::Event::JoystickButtonPressed);

                m_luaArguments->PushArgument(static_cast<int>(m_event.joystickButton.joystickId));
                m_luaArguments->PushArgument(static_cast<int>(m_event.joystickButton.button));
                m_luaArguments->PushArgument(m_event.type == sf::Event::JoystickButtonPressed ? 1 : 0);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onJoypadButton, m_luaArguments);
                m_luaArguments->Clear();
            } break;
            case sf::Event::JoystickMoved:
            {
                if(m_joypadAxisCallback) (*m_joypadAxisCallback)(m_event.joystickButton.joystickId, m_event.joystickMove.axis, m_event.joystickMove.position);

                m_luaArguments->PushArgument(static_cast<int>(m_event.joystickMove.joystickId));
                m_luaArguments->PushArgument(g_JoypadAxisNamesTable[m_event.joystickMove.axis]);
                m_luaArguments->PushArgument(m_event.joystickMove.position);
                m_core->GetLuaManager()->GetEventManager()->CallEvent(EventManager::EME_onJoypadAxis, m_luaArguments);
                m_luaArguments->Clear();
            } break;
        }
    }
    return m_active;
}
