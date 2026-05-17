#pragma once
#include <string>
#include <vector>
#include <imGuiWindows.h>

namespace utils
{
    static bool DrawStringProperty(const char* _label, std::string& _value, size_t _bufferSize = 512)
    {
        std::vector<char> buffer(_bufferSize, '\0');

        std::snprintf(buffer.data(), buffer.size(), "%s", _value.c_str());

        if (ImGui::InputText(_label, buffer.data(), buffer.size()))
        {
            _value = buffer.data();
            return true;
        }

        return false;
    }
}