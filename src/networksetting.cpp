//
// Created by gzmorell on 23/03/25.
//

#include "networksetting.h"
#include "ftxui-ip-input/ipv4.hpp"
#include "ftxui/dom/table.hpp"
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#include <iostream>

NetworkSetting::NetworkSetting()
{
    load();
    m_address_input = InputIPV4(&m_address);
    m_mask_input = InputIPV4(&m_mask);
    m_gateway_input = InputIPV4(&m_gateway);
    m_save_button = Button(
        "Save",
        [this] {
            auto ok = save();
            if (ok) {
                std::cout << "ok" << std::endl;
            } else {
                std::cout << "error" << std::endl;
            }
        },
        ButtonOption::Animated());
    m_layout = Container::Vertical(
        {Container::Vertical({m_address_input, m_mask_input, m_gateway_input}), m_save_button});
}
void NetworkSetting::load()
{
    std::fstream configFile;
    std::filesystem::path Path(R"(/etc/network/interfaces)");
    std::string data;
    // std::cout << "Trying to open the file" << std::endl;
    configFile.open(Path, std::ios::in);
    if (configFile.is_open()) {
        std::stringstream ss;
        ss << configFile.rdbuf();
        data = ss.str();
    } else
        return;
    configFile.close();
    std::regex addressPattern(R"(address ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}))");
    std::regex maskPattern(R"(mask ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}))");
    std::regex gatewayPattern(R"(gateway ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}))");
    std::regex interfacePattern("iface (.*) inet static");
    std::smatch match;
    if (std::regex_search(data, match, addressPattern)) {
        if (match.size() > 1)
            m_address = match[1];
    };
    if (std::regex_search(data, match, maskPattern)) {
        if (match.size() > 1)
            m_mask = match[1];
    };
    if (std::regex_search(data, match, gatewayPattern)) {
        if (match.size() > 1)
            m_gateway = match[1];
    };
    if (std::regex_search(data, match, interfacePattern)) {
        if (match.size() > 1)
            m_interface = match[1];
    };
}
bool NetworkSetting::save() const
{
    std::filesystem::path Path(R"(/etc/network/interfaces)");
    std::fstream configFile(Path, std::ios::in);
    std::string data;
    if (configFile.is_open()) {
        std::stringstream ss;
        ss << configFile.rdbuf();
        data = ss.str();
    } else
        return false;
    configFile.close();
    std::regex addressPattern(R"(address [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})");
    std::regex maskPattern(R"(mask [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})");
    std::regex gatewayPattern(R"(gateway [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})");
    std::string address = "address " + m_address;
    std::string mask = "mask " + m_mask;
    std::string gateway = "gateway " + m_gateway;
    data = std::regex_replace(data, addressPattern, address);
    data = std::regex_replace(data, maskPattern, mask);
    data = std::regex_replace(data, gatewayPattern, gateway);

    configFile.open(Path, std::ios::out | std::ios::trunc);
    configFile.seekg(0, std::ios::beg);
    if (configFile.is_open()) {
        configFile << data;
    } else
        return false;
    configFile.close();
    return true;
}

Component NetworkSetting::Render()
{
    auto renderer = Renderer(m_layout, [this] {
        std::vector<std::vector<Element>> table_content;
        table_content.push_back({text("Parameter"), text("Value"), text("New Value")});
        table_content.push_back({text("Interface"), text(m_interface), text("")});
        table_content.push_back({text("Address"), text(m_address), m_address_input->Render()});
        table_content.push_back({text("Mask"), text(m_mask), m_mask_input->Render()});
        table_content.push_back({text("Gateway"), text(m_gateway), m_gateway_input->Render()});
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