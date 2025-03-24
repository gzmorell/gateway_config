//
// Created by gzmorell on 23/03/25.
//

#include "rcsusetting.h"
#include "ftxui-ip-input/ipv4.hpp"
#include "ftxui/dom/table.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "util.h"

using json = nlohmann::json;

RcsuSetting::RcsuSetting()
{
    m_rca_modes = {"Listen", "Copy", "Command", "Virtual"};
    load();
    m_rca_address_input = InputIPV4(&m_rca_address);
    m_rca_local_port_input = Input(&m_rca_local_port);
    m_rca_remote_port_input = Input(&m_rca_remote_port);
    m_tca_address_input = InputIPV4(&m_tca_address);
    m_tca_local_port_input = Input(&m_tca_local_port);
    m_tca_remote_port_input = Input(&m_tca_remote_port);
    m_rca_mode_drop = Dropdown(&m_rca_modes, &m_rca_selected_mode);
    m_save_button = Button(
        "Save",
        [this] {
            if (const auto ok = save()) {
                nested("Config saved!", ok);
            } else {
                nested("Error saving!", ok);
            }
        },
        ButtonOption::Animated());
    m_layout = Container::Vertical(
        {Container::Vertical({
             m_rca_address_input,
             m_rca_local_port_input,
             m_rca_remote_port_input,
             m_rca_mode_drop,
             m_tca_address_input,
             m_tca_local_port_input,
             m_tca_remote_port_input,
         }),
         m_save_button});
}
bool RcsuSetting::load()
{
    std::filesystem::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    bool ok = true;
    try {
        auto data = json::parse(configFile);
        try {
            m_rca_address = data["Rca"]["IpAddress"];
        } catch (...) {
            ok = false;
        }
        try {
            int port;
            port = data["Rca"]["LocalPort"];
            m_rca_local_port = std::to_string(port);
        } catch (...) {
            ok = false;
        }
        try {
            int port;
            port = data["Rca"]["RemotePort"];
            m_rca_remote_port = std::to_string(port);
        } catch (...) {
            ok = false;
        }
        try {
            m_tca_address = data["Tca"]["IpAddress"];
        } catch (...) {
            ok = false;
        }
        try {
            int port;
            port = data["Tca"]["LocalPort"];
            m_tca_local_port = std::to_string(port);
        } catch (...) {
            ok = false;
        }
        try {
            int port;
            port = data["Tca"]["RemotePort"];
            m_tca_remote_port = std::to_string(port);
        } catch (...) {
            ok = false;
        }
        try {
            std::string mode = data["Rca"]["Mode"];
            if (auto it = std::find(m_rca_modes.begin(), m_rca_modes.end(), mode);
                it == m_rca_modes.end()) {
                m_rca_selected_mode = 0;
            } else {
                m_rca_selected_mode = static_cast<int>(it - m_rca_modes.begin());
            }
        } catch (...) {
            ok = false;
        }
    } catch (...) {
        ok = false;
    }
    return ok;
}

bool RcsuSetting::save() const
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
        data["Rca"]["IpAddress"] = m_rca_address;
    } catch (...) {
        ok = false;
    }
    try {
        int port;
        port = std::stoi(m_rca_local_port);
        data["Rca"]["LocalPort"] = port;
    } catch (...) {
        ok = false;
    }
    try {
        int port;
        port = std::stoi(m_rca_remote_port);
        data["Rca"]["RemotePort"] = port;
    } catch (...) {
        ok = false;
    }
    try {
        data["Tca"]["IpAddress"] = m_tca_address;
    } catch (...) {
        ok = false;
    }
    try {
        int port;
        port = std::stoi(m_tca_local_port);
        data["Tca"]["LocalPort"] = port;
    } catch (...) {
        ok = false;
    }
    try {
        int port;
        port = std::stoi(m_tca_remote_port);
        data["Tca"]["RemotePort"] = port;
    } catch (...) {
        ok = false;
    }
    try {
        std::string mode = m_rca_modes[m_rca_selected_mode];
        data["Rca"]["Mode"] = mode;
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

Component RcsuSetting::Render() const
{
    auto renderer = Renderer(m_layout, [this] {
        std::vector<std::vector<Element>> table_content;
        table_content.push_back({text("Parameter"), text("Value"), text("New Value")});
        table_content.push_back(
            {text("RCA Address"), text(m_rca_address), m_rca_address_input->Render()});
        table_content.push_back({text("Rca Local Port"), text(m_rca_local_port), m_rca_local_port_input->Render()});
        table_content.push_back({text("Rca Remote Port"), text(m_rca_remote_port), m_rca_remote_port_input->Render()});
        table_content.push_back({text("Rca Mode"), text(m_rca_modes[m_rca_selected_mode]), m_rca_mode_drop->Render()});
        table_content.push_back(
            {text("TCA Address"), text(m_tca_address), m_tca_address_input->Render()});
        table_content.push_back({text("Tca Local Port"), text(m_tca_local_port), m_tca_local_port_input->Render()});
        table_content.push_back({text("Tca Remote Port"), text(m_tca_remote_port), m_tca_remote_port_input->Render()});
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
