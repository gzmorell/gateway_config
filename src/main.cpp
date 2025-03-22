// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
// #include <stddef.h>    // for size_t
#include <array> // for array
// #include <chrono>      // for operator""s, chrono_literals
// #include <cmath>       // for sin
#include <functional> // for ref, reference_wrapper, function
// #include <memory>      // for allocator, shared_ptr, __shared_ptr_access
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string> // for string, basic_string, char_traits, operator+, to_string
// #include <string>
#include <utility> // for move
#include <vector>  // for vector

#include "ftxui/component/component.hpp" // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase, Component
#include "ftxui/component/component_options.hpp"  // for MenuOption, InputOption
#include "ftxui/component/event.hpp"              // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/table.hpp"
// #include "ftxui/dom/flexbox_config.hpp"  // for FlexboxConfig
#include "ftxui/screen/color.hpp" // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
// #include "ftxui/screen/color_info.hpp"  // for ColorInfo
// #include "ftxui/screen/terminal.hpp"    // for Size, Dimensions
#include "subprocess.hpp"

using namespace ftxui;
using namespace subprocess::literals;
using json = nlohmann::json;
namespace fs = std::filesystem;

const auto button_style = ButtonOption::Animated();

// trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(
        std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        s.end());
}

inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

struct NetWorkConfig
{
    std::string interface = "eth0";
    std::string address = "192.168.1.158";
    std::string netmask = "255.255.255.0";
    std::string gateway = "192.168.1.1";
};

struct GatewayConfig
{
    std::string version;
    std::string ils_address = "192.168.1.156";
    int ils_port = 3001;
    std::string master_password = "MASTER1 MASTER2 MASTER3";
    std::string user_name = "GATEWAY";
    std::string user_password = "GATEWAY1 GATEWAY2 GATEWAY3";
    int server_port = 3001;
    std::string server_password = "nolosabeNadie!";
    std::string priority_address = "0.0.0.0";
    std::vector<std::string> trap_ips = {"10.0.0.2"};
};

void load_network_config(NetWorkConfig &config)
{
    std::fstream configFile;
    fs::path Path(R"(/etc/network/interfaces)");
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
            config.address = match[1];
    };
    if (std::regex_search(data, match, maskPattern)) {
        if (match.size() > 1)
            config.netmask = match[1];
    };
    if (std::regex_search(data, match, gatewayPattern)) {
        if (match.size() > 1)
            config.gateway = match[1];
    };
    if (std::regex_search(data, match, interfacePattern)) {
        if (match.size() > 1)
            config.interface = match[1];
    };
}

void load_gateway_config(GatewayConfig &gw_config)
{
    fs::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    // std::cout << "Trying to open the file" << std::endl;
    try {
        auto data = json::parse(configFile);
        try {
            std::string ils_address;
            ils_address = data["Ils"]["IpAddress"];
            gw_config.ils_address = ils_address;
        } catch (...) {
        }
        try {
            int ils_port;
            ils_port = data["Ils"]["Port"];
            gw_config.ils_port = ils_port;
        } catch (...) {
        }
        try {
            int server_port;
            server_port = data["Server"]["Port"];
            gw_config.server_port = server_port;
        } catch (...) {
        }
        try {
            std::string user_name_encrypted = data["Ils"]["User"]["Name"];
            std::string user_name;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + user_name_encrypted};
            cmd > user_name;
            cmd >= caca;
            try {
                cmd.run();
                trim(user_name);
                gw_config.user_name = user_name;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::vector<std::string> trap_ips;
            trap_ips = data["TrapIps"];
            gw_config.trap_ips = trap_ips;
        } catch (...) {
        }
        try {
            std::string version;
            version = data["Version"];
            gw_config.version = version;
        } catch (...) {
        }
        try {
            std::string priority_address;
            priority_address = data["Server"]["PriorityAddress"];
            gw_config.priority_address = priority_address;
        } catch (...) {
        }
        try {
            std::string user_password_encrypted = data["Ils"]["User"]["Passwords"];
            std::string user_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + user_password_encrypted};
            cmd > user_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(user_password);
                gw_config.user_password = user_password;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::string master_password_encrypted = data["Ils"]["Master"];
            std::string master_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + master_password_encrypted};
            cmd > master_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(master_password);
                gw_config.master_password = master_password;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::string server_password_encrypted = data["Server"]["Password"];
            std::string server_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + server_password_encrypted};
            cmd > server_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(server_password);
                gw_config.server_password = server_password;
            } catch (...) {
            }
        } catch (...) {
        }
    } catch (...) {
        return;
    }
}

namespace ftxui {

class ScrollerBase : public ComponentBase
{
public:
    explicit ScrollerBase(const Component &child) { Add(child); }

private:
    Element Render() final
    {
        auto focused = Focused() ? focus : ftxui::select;
        auto style = Focused() ? inverted : nothing;

        Element background = ComponentBase::Render();
        background->ComputeRequirement();
        size_ = background->requirement().min_y;
        return dbox({
                   std::move(background),
                   vbox({
                       text(L"") | size(HEIGHT, EQUAL, selected_),
                       text(L"") | style | focused,
                   }),
               })
               | vscroll_indicator | yframe | yflex | reflect(box_);
    }

    bool OnEvent(Event event) final
    {
        if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y))
            TakeFocus();

        int selected_old = selected_;
        if (event == Event::ArrowUp || event == Event::Character('k')
            || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
            selected_--;
        }
        if ((event == Event::ArrowDown || event == Event::Character('j')
             || (event.is_mouse() && event.mouse().button == Mouse::WheelDown))) {
            selected_++;
        }
        if (event == Event::PageDown)
            selected_ += box_.y_max - box_.y_min;
        if (event == Event::PageUp)
            selected_ -= box_.y_max - box_.y_min;
        if (event == Event::Home)
            selected_ = 0;
        if (event == Event::End)
            selected_ = size_;

        selected_ = std::max(0, std::min(size_ - 1, selected_));
        return selected_old != selected_;
    }

    [[nodiscard]] bool Focusable() const final { return true; }

    int selected_ = 0;
    int size_ = 0;
    Box box_;
};

Component Logger(const Component& child)
{
    return Make<ScrollerBase>(child);
}
} // namespace ftxui

Component network_config()
{
    auto renderer = Renderer([=] {
        NetWorkConfig net_config;
        load_network_config(net_config);
        std::vector<std::vector<std::string>> table_content;
        table_content.push_back({"Parameter", "Value"});
        table_content.push_back({"Interface", net_config.interface});
        table_content.push_back({"Address", net_config.address});
        table_content.push_back({"Mask", net_config.netmask});
        table_content.push_back({"Gateway", net_config.gateway});
        auto table = Table(table_content);

        table.SelectAll().Border(LIGHT);

        // Add border around the first column.
        table.SelectColumn(0).Border(LIGHT);

        // Make first row bold with a double border.
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).SeparatorVertical(LIGHT);
        table.SelectRow(0).Border(DOUBLE);

        // Align right the "Release date" column.
        table.SelectColumn(1).DecorateCells(align_right);

        // Select row from the second to the last.
        auto content = table.SelectRows(1, -1);
        // Alternate in between 3 colors.
        content.DecorateCellsAlternateRow(color(Color::Blue), 2, 0);
        content.DecorateCellsAlternateRow(color(Color::Cyan), 2, 1);
        auto tb = vbox(std::move(table.Render()));

        return tb;
    });
    //Renderer([=] { return nonWrappingParagraph(str); });
    return renderer;
}

Component gateway_config()
{
    auto renderer = Renderer([=] {
        GatewayConfig gw_config;
        load_gateway_config(gw_config);
        std::vector<std::vector<std::string>> table_content;
        table_content.push_back({"Parameter", "Value"});
        table_content.push_back({"Ils Address", gw_config.ils_address});
        table_content.push_back({"Ils Port", std::to_string(gw_config.ils_port)});
        table_content.push_back({"Server Port", std::to_string(gw_config.server_port)});
        table_content.push_back({"User Name", gw_config.user_name});
        for (const auto& ip : gw_config.trap_ips) {
            table_content.push_back({"Trap IP", ip});
        }
        auto table = Table(table_content);

        table.SelectAll().Border(LIGHT);

        // Add border around the first column.
        table.SelectColumn(0).Border(LIGHT);

        // Make first row bold with a double border.
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).SeparatorVertical(LIGHT);
        table.SelectRow(0).Border(DOUBLE);

        // Align right the "Release date" column.
        table.SelectColumn(1).DecorateCells(align_right);

        // Select row from the second to the last.
        auto content = table.SelectRows(1, -1);
        // Alternate in between 3 colors.
        content.DecorateCellsAlternateRow(color(Color::Blue), 2, 0);
        content.DecorateCellsAlternateRow(color(Color::Cyan), 2, 1);
        auto tb = vbox(std::move(table.Render()));

        return tb;
    });
    //Renderer([=] { return nonWrappingParagraph(str); });
    return renderer;
}
Component ModalComponent(
    std::function<void()> do_action, std::function<void()> do_cancel, const std::string &action_text)
{
    auto leftButton = Button(action_text, std::move(do_action), button_style);
    auto rightButton = Button(" Cancel ", std::move(do_cancel), button_style);
    auto component = Container::Vertical({leftButton, rightButton});
    // Polish how the two buttons are rendered
    component |= Renderer([&](const Element &inner) {
        FlexboxConfig config;
        config.justify_content = FlexboxConfig::JustifyContent::SpaceBetween;
        return vbox(
                   {text("Confirm Action") | center,
                    separator(),
                    flexbox(
                        {
                            inner | xflex_grow,
                        },
                        config)})
               | size(WIDTH, GREATER_THAN, 30) | border;
    });
    return component;
}

Elements Split(const std::string& the_text)
{
    Elements output;
    std::stringstream ss(the_text);
    std::string word;
    while (std::getline(ss, word, '\n'))
        output.push_back(text(word));
    return output;
}

Element logRender(const std::string& the_text)
{
    Elements lines;
    for (auto &line : Split(the_text))
        lines.push_back(line);
    return vbox(std::move(lines));
}

Component DummyWindowContent()
{
    class Impl : public ComponentBase
    {
    private:
        float scroll_x = 0.1;
        float scroll_y = 0.1;

    public:
        Impl()
        {
            std::string str;
            try {
                ("cat /var/log/messages"_cmd | "grep rms"_cmd > str).run();
            } catch (...) {
                str = "No messages";
            }
            auto content = Renderer([=] { return logRender(str); });

            auto scrollable_content = Renderer(content, [&, content] {
                return content->Render() | focusPositionRelative(scroll_x, scroll_y) | frame | flex;
            });

            SliderOption<float> option_x;
            option_x.value = &scroll_x;
            option_x.min = 0.f;
            option_x.max = 1.f;
            option_x.increment = 0.1f;
            option_x.direction = Direction::Right;
            option_x.color_active = Color::Blue;
            option_x.color_inactive = Color::BlueLight;
            auto scrollbar_x = Slider(option_x);

            SliderOption<float> option_y;
            option_y.value = &scroll_y;
            option_y.min = 0.f;
            option_y.max = 1.f;
            option_y.increment = 0.1f;
            option_y.direction = Direction::Down;
            option_y.color_active = Color::Yellow;
            option_y.color_inactive = Color::YellowLight;
            auto scrollbar_y = Slider(option_y);

            Add(Container::Vertical({
                Container::Horizontal({
                    scrollable_content,
                    scrollbar_y,
                }) | flex,
                Container::Horizontal({
                    scrollbar_x,
                    Renderer([] { return text(L"x"); }),
                }),
            }));

            Add(Container::Vertical({content}));
        }
    };
    return Make<Impl>();
}

int main()
{
    auto screen = ScreenInteractive::Fullscreen();
    std::string password_text;
    int password_count = 0;
    fs::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    std::string password_plain;
    auto data = json::parse(configFile);

    try {
        std::string password_encrypted = data["Server"]["Password"];
        std::string decrypt = "setup-gw -d ";
        std::string caca;
        auto cmd = subprocess::command{decrypt + password_encrypted};
        cmd > password_plain;
        cmd >= caca;

        try {
            cmd.run();
            trim(password_plain);
        } catch (...) {
            password_plain = "nolosabeNadie!";
        }
    } catch (...) {
        password_plain = "nolosabeNadie!";
    }

    // int shift = 0;
    bool shown_exit = false;
    bool shown_reboot = false;
    bool shown_save = false;
    bool shown_login = false;
    bool system_logged = false;

    auto run_login = [&] {
        if (password_text == password_plain) {
            system_logged = true;
            password_count = 0;
            screen.Exit();
        } else {
            if (password_count > 3)
                screen.Exit();
            else
                ++password_count;
        }
        password_text = "";
    };
    auto run_exit = [&] { screen.Exit(); };
    auto run_reboot = [&] {
        subprocess::command cmd{"sudo /sbin/reboot"};
        try {
            cmd.run();
        } catch (...) {
            return;
        }
        shown_save = false;
    };
    auto run_save = [&] {
        subprocess::command cmd{"sudo /sbin/lbu ci"};
        try {
            cmd.run();
        } catch (...) {
            return;
        }
        shown_save = false;
    };
    auto cancel_exit = [&] { shown_exit = false; };
    auto show_exit = [&] { shown_exit = true; };
    auto cancel_reboot = [&] { shown_reboot = false; };
    auto show_reboot = [&] { shown_reboot = true; };
    auto cancel_save = [&] { shown_save = false; };
    auto show_save = [&] { shown_save = true; };
    auto cancel_login = [&] {
        shown_login = false;
        screen.Exit();
    };
    auto show_login = [&] { shown_login = true; };

    const std::string &label_exit = "Exit";
    const std::string &label_reboot = "Reboot";
    const std::string &label_save = "Save";
    const std::string &label_login = " Login ";

    auto component_exit = ModalComponent(run_exit, cancel_exit, label_exit);
    auto component_reboot = ModalComponent(run_reboot, cancel_reboot, label_reboot);
    auto component_save = ModalComponent(run_save, cancel_save, label_save);

    //// LOGIN

    auto login_button = Button(label_login, run_login, button_style);
    auto cancel_login_button = Button(" Cancel ", cancel_login, button_style);
    auto label_password = text("Password");
    auto password_input_option = InputOption();
    password_input_option.password = true;
    password_input_option.multiline = false;
    password_input_option.on_enter = run_login;
    auto input_password = Input(&password_text, "", password_input_option);
    auto password_component = Container::Vertical(
        {Container::Horizontal({input_password}),
         Container::Vertical({login_button, cancel_login_button})});
    auto login_renderer = Renderer(password_component, [&] {
        auto input_win = window(
            text("Login"),
            vbox(
                {vbox({
                     filler(),
                     hbox({
                         text("Password: "),
                         input_password->Render(),
                     }),
                     filler(),
                 }) | size(WIDTH, EQUAL, 30)
                     | size(HEIGHT, EQUAL, 3),
                 separator(),
                 hbox({
                     separatorEmpty() | flex,
                     login_button->Render() | size(WIDTH, EQUAL, 10),
                     separatorEmpty(),
                     cancel_login_button->Render() | size(WIDTH, EQUAL, 10),
                     separatorEmpty() | flex,
                 })}));
        return vbox({
                   hbox({
                       input_win,
                       filler(),
                   }) | size(HEIGHT, LESS_THAN, 12),
               })
               | flex_grow;
    });

    //// TABLE

    std::string value_1;
    std::string value_2;
    std::string value_3;
    std::string value_4;

    // auto input_1 = Input(value_1, "input_1");
    // auto input_2 = Input(value_2, "input_2");
    auto text_1 = Renderer([] { return text("text 1");});
    auto text_2 = Renderer([] { return text("text 2");});
    auto input_1 = Input(value_3, "input_1");
    auto input_2 = Input(value_4, "input_2");

    // Layout combine the components above, so that user can navigate them using
    // arrow keys in both directions. Unfortunately, ftxui doesn't provide a
    // Container::Array component, so this is a best effort here:
    auto layout = Container::Vertical({
        Container::Horizontal({text_1, input_1}),
        Container::Horizontal({text_2, input_2}),
    });

    // Renderer override the `ComponentBase::Render` function of `layout`, to
    // display them using a Table.
    auto table_renderer = Renderer(layout, [&] {
        auto table = Table({
            {text(""), text("Present Value"), text("New Value")},
            {text("Network Address"), input_1->Render(), input_2->Render()},
            {text("Network Mask"), text_1->Render(), input_1->Render()},
            {text("Network Gateway "), text_2->Render(), input_2->Render()},
        });
        table.SelectAll().Border(DOUBLE);
        table.SelectAll().Separator(LIGHT);

        return table.Render();
    });

    //     // ---------------------------------------------------------------------------
    // Tabs
    // ---------------------------------------------------------------------------
    //

    auto log_renderer = Renderer([] {
        std::string str;
        try {
            ("cat /var/log/messages"_cmd | "grep rms"_cmd > str).run();
        } catch (...) {
            str = "No messages";
        }
        return logRender(str);
    });

    auto logger = Logger(log_renderer);

    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        "Network",
        "Gateway",
        "Config",
        "Logs",
    };
    auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
    auto tab_content = Container::Tab(
        {
            network_config(),
            gateway_config(),
            table_renderer,
            logger,
        },
        &tab_index);

    auto button_exit = Button(label_exit, show_exit, ButtonOption::Animated());
    auto button_reboot = Button(label_reboot, show_reboot, ButtonOption::Animated());
    auto button_save = Button(label_save, show_save, ButtonOption::Animated());
    auto button_login = Button(label_login, show_login, ButtonOption::Animated());

    auto main_container = Container::Vertical({
        Container::Horizontal({
            tab_selection,
            button_save,
            button_reboot,
            button_exit,
        }),
        tab_content,
    });
    auto login_container = Container::Horizontal({
        button_login,
    });

    auto main_renderer = Renderer(main_container, [&] {
        return vbox({
            text("Gateway Configuration") | bold | hcenter,
            hbox({
                tab_selection->Render() | flex,
                separatorEmpty(),
                button_save->Render(),
                separatorEmpty(),
                button_reboot->Render(),
                separatorEmpty(),
                button_exit->Render(),
            }),
            tab_content->Render() | flex,
        });
    });
    auto logout_renderer = Renderer(login_container, [&] {
        return vbox({
            text("Gateway Login") | bold | hcenter,
            hbox({
                separatorEmpty() | flex,
                button_login->Render(),
            }),
        });
    });
    main_renderer |= Modal(component_save, &shown_save);
    main_renderer |= Modal(component_reboot, &shown_reboot);
    main_renderer |= Modal(component_exit, &shown_exit);
    logout_renderer = logout_renderer |= Modal(login_renderer, &shown_login);

    screen.Loop(logout_renderer);
    screen.Clear();
    if (system_logged)
        screen.Loop(main_renderer);
    return 0;
}
