//
// Created by gzmorell on 23/03/25.
//

#ifndef RCSUSETTING_H
#define RCSUSETTING_H

#include <string>
#include "ftxui/component/component.hpp"

using namespace ftxui;

class RcsuSetting {
private:
    std::string m_rca_address;
    std::string m_rca_local_port;
    std::string m_rca_remote_port;
    std::string m_tca_address;
    std::string m_tca_local_port;
    std::string m_tca_remote_port;

    Component m_rca_address_input;
    Component m_rca_local_port_input;
    Component m_rca_remote_port_input;
    Component m_rca_mode_drop;
    Component m_tca_address_input;
    Component m_tca_local_port_input;
    Component m_tca_remote_port_input;
    Component m_layout;
    Component m_save_button;
    std::vector<std::string> m_rca_modes;
    int m_rca_selected_mode = 0;

    public:
    RcsuSetting();
    ~RcsuSetting()= default;

    bool load();
    [[nodiscard]] bool save() const;
    [[nodiscard]] Component Render() const;

};

#endif //RCSUSETTING_H
