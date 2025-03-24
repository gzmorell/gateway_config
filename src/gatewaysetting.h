//
// Created by gzmorell on 23/03/25.
//

#ifndef GATEWAYSETTING_H
#define GATEWAYSETTING_H

#include <string>
#include "ftxui/component/component.hpp"

using namespace ftxui;
constexpr std::size_t MAX_TRAP_IPS = 5;

class GatewaySetting {
private:
    std::string m_gateway_version;
    std::string m_ils_address;
    std::string m_ils_port;
    std::string m_rmm_server_port;
    std::string m_rmm_priority_address;
    std::string m_trap_ip[MAX_TRAP_IPS];

    Component m_ils_address_input;
    Component m_ils_port_input;
    Component m_rmm_server_port_input;
    Component m_rmm_priority_address_input;
    Component m_trap_ip_input[MAX_TRAP_IPS];
    Component m_layout;
    Component m_save_button;

    public:
    GatewaySetting();
    ~GatewaySetting()= default;
    bool erased = false;

    bool load();
    [[nodiscard]] bool save() ;
    [[nodiscard]] Component Render() const;

};

#endif //GATEWAYSETTING_H
