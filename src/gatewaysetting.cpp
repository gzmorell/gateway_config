//
// Created by gzmorell on 23/03/25.
//

#include "gatewaysetting.h"
#include "ftxui-ip-input/ipv4.hpp"
#include "ftxui/dom/table.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <iostream>
#include "util.h"

using json = nlohmann::json;

GatewaySetting::GatewaySetting()
{
    load();
    m_ils_address_input = InputIPV4(&m_ils_address);
    m_rmm_priority_address_input = InputIPV4(&m_rmm_priority_address);
    for (int i = 0; i < MAX_TRAP_IPS; ++i)
        m_trap_ip_input[i] = InputIPV4(&m_trap_ip[i]);
    m_ils_port_input = Input(&m_ils_port);
    m_rmm_server_port_input = Input(&m_rmm_server_port);
    m_save_button = Button(
        "Save",
        [this] {
            if (auto ok = save()) {
                nested("Configuration Saved", ok);
            } else {
                nested("Error Saving Configuration !", ok);
            }
        },
        ButtonOption::Animated());
    m_layout = Container::Vertical(
        {Container::Vertical({
             m_ils_address_input,
             m_ils_port_input,
             m_rmm_server_port_input,
             m_rmm_priority_address_input,
             m_trap_ip_input[0],
             m_trap_ip_input[1],
             m_trap_ip_input[2],
             m_trap_ip_input[3],
             m_trap_ip_input[4],
         }),
         m_save_button});
}
bool GatewaySetting::load()
{
    std::filesystem::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    bool ok = true;
    try {
        auto data = json::parse(configFile);
        try {
            m_ils_address = data["Ils"]["IpAddress"];
        } catch (...) {
            ok = false;
        }
        try {
            int ils_port;
            ils_port = data["Ils"]["Port"];
            m_ils_port = std::to_string(ils_port);
        } catch (...) {
            ok = false;
        }
        try {
            int server_port;
            server_port = data["Server"]["Port"];
            m_rmm_server_port = std::to_string(server_port);
        } catch (...) {
            ok = false;
        }
        try {
            std::vector<std::string> trap_ips;
            trap_ips = data["TrapIps"];
            auto count = trap_ips.size();
            count = std::min(count, MAX_TRAP_IPS);
            for (size_t i = 0; i < count; ++i) {
                m_trap_ip[i] = trap_ips[i];
            }
        } catch (...) {
            ok = false;
        }
        try {
            std::string version;
            version = data["Version"];
            m_gateway_version = version;
        } catch (...) {
            ok = false;
        }
        try {
            std::string priority_address;
            priority_address = data["Server"]["PriorityAddress"];
            m_rmm_priority_address = priority_address;
        } catch (...) {
            ok = false;
        }
    } catch (...) {
        ok = false;
    }
    return ok;
}

bool GatewaySetting::save()
{
    std::filesystem::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path, std::fstream::in);
    bool ok = true;
    if (!configFile.is_open()) {
        return false;
    }
    json data;
    try {
        data = json::parse(configFile);
    } catch (...) {
        return false;
    }
    configFile.close();
    try {
        data["Ils"]["IpAddress"] = m_ils_address;
    } catch (...) {
        ok = false;
    }
    try {
        int ils_port;
        ils_port = std::stoi(m_ils_port);
        data["Ils"]["Port"] = ils_port;
    } catch (...) {
        ok = false;
    }
    try {
        int server_port;
        server_port = std::stoi(m_rmm_server_port);
        data["Server"]["Port"] = server_port;
    } catch (...) {
        ok = false;
    }
    try {
        auto trap_size = data["TrapIps"].size();
        trap_size = std::min(trap_size, MAX_TRAP_IPS) - 1;
        for (auto i = 0; i < MAX_TRAP_IPS; ++i) {
            if (auto ip = m_trap_ip[i]; ip.empty() || ip == "0.0.0.0") {
                for (auto j = trap_size; j >= i; --j) {
                    m_trap_ip[j] = "0.0.0.0";
                    data["TrapIps"].erase(j);
                }
                break;
            } else {
                data["TrapIps"][i] = ip;
            }
        }
    } catch (...) {
        ok = false;
    }
    try {
        data["Server"]["PriorityAddress"] = m_rmm_priority_address;
    } catch (...) {
        ok = false;
    }
    auto j = data.dump(4);

    configFile.open(Path, std::ios::out | std::ios::trunc);
    if (!configFile.is_open()) {
        return false;
    }
    configFile << j;
    configFile.close();
    return ok;
}

Component GatewaySetting::Render() const
{
    auto renderer = Renderer(m_layout, [this] {
        std::vector<std::vector<Element>> table_content;
        table_content.push_back({text("Parameter"), text("Value"), text("New Value")});
        table_content.push_back({text("Version"), text(m_gateway_version), text("")});
        table_content.push_back(
            {text("ILS Address"), text(m_ils_address), m_ils_address_input->Render()});
        table_content.push_back({text("ILS Port"), text(m_ils_port), m_ils_port_input->Render()});
        table_content.push_back(
            {text("RMM Sever Port"), text(m_rmm_server_port), m_rmm_server_port_input->Render()});
        table_content.push_back(
            {text("RMM Sever Priority Address"),
             text(m_rmm_priority_address),
             m_rmm_priority_address_input->Render()});
        for (std::size_t i = 0; i < MAX_TRAP_IPS; ++i) {
            table_content.push_back(
                {text("Trap IP"),
                 text(m_trap_ip[i]),
                 m_trap_ip_input[i]->Render()});
        }
        auto table = Table(table_content);
        table.SelectAll().Border(LIGHT);
        // Add border around the first column.
        table.SelectColumn(0).Border(LIGHT);
        table.SelectColumn(1).Border(LIGHT);

        // Make first row bold with a double border.
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).SeparatorVertical(LIGHT);
        table.SelectRow(0).Border(DOUBLE);

        // Align right the "Release date" column.
        // table.SelectColumn(1).DecorateCells(align_right);

        // Select row from the second to the last.
        auto content = table.SelectRows(1, -1);
        // Alternate in between 3 colors.
        content.DecorateCellsAlternateRow(color(Color::Blue), 2, 0);
        content.DecorateCellsAlternateRow(color(Color::Cyan), 2, 1);
        auto tb = vbox(
            std::move(table.Render()),
            hbox(separatorEmpty(), m_save_button->Render(), separatorEmpty() | flex));
        return tb;
    });
    return renderer;
}