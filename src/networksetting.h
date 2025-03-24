//
// Created by gzmorell on 23/03/25.
//

#ifndef NETWORKSETTING_H
#define NETWORKSETTING_H

#include <string>
#include "ftxui/component/component.hpp"

using namespace ftxui;

class NetworkSetting {
private:
    std::string m_interface;
    std::string m_address;
    std::string m_mask;
    std::string m_gateway;
    Component m_address_input;
    Component m_mask_input;
    Component m_gateway_input;
    Component m_layout;
    Component m_save_button;

    public:
    NetworkSetting();
    ~NetworkSetting() = default;

    void load();
    [[nodiscard]] bool save() const;
    Component Render();
    std::string address() { return m_address; }
    std::string mask() { return m_mask; }
    std::string gateway() { return m_gateway; }

};

#endif //NETWORKSETTING_H
