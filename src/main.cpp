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
#include <string>
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

namespace ftxui {

class ScrollerBase : public ComponentBase
{
public:
    ScrollerBase(Component child) { Add(child); }

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

    bool Focusable() const final { return true; }

    int selected_ = 0;
    int size_ = 0;
    Box box_;
};

Component Scroller(Component child)
{
    return Make<ScrollerBase>(std::move(child));
}
} // namespace ftxui

Component NetworkConfig()
{
    auto renderer = Renderer([=] {
        std::fstream configFile;
        fs::path Path(R"(/etc/network/interfaces)");
        std::string data;
        std::stringstream ss;
        std::string address;
        std::string mask;
        std::string gateway;
        std::string interface;
        // std::cout << "Trying to open the file" << std::endl;
        configFile.open(Path, std::ios::in);
        if (configFile.is_open()) {
            ss << configFile.rdbuf();
            data = ss.str();
        }
        configFile.close();
        std::regex addressPattern("address ([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
        std::regex maskPattern("mask ([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
        std::regex gatewayPattern("gateway ([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
        std::regex interfacePattern("iface (.*) inet static");
        std::smatch match;
        if (std::regex_search(data, match, addressPattern)) {
            if (match.size() > 1)
                address = match[1];
        };
        if (std::regex_search(data, match, maskPattern)) {
            if (match.size() > 1)
                mask = match[1];
        };
        if (std::regex_search(data, match, gatewayPattern)) {
            if (match.size() > 1)
                gateway = match[1];
        };
        if (std::regex_search(data, match, interfacePattern)) {
            if (match.size() > 1)
                interface = match[1];
        };

        std::vector<std::vector<std::string>> table_content;
        table_content.push_back({"Parameter", "Value"});
        table_content.push_back({"Interface", interface});
        table_content.push_back({"Address", address});
        table_content.push_back({"Mask", mask});
        table_content.push_back({"Gateway", address});
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

Component GatewayConfig()
{
    auto renderer = Renderer([=] {
        fs::path Path(R"(/etc/pasarela/settings.json)");
        std::fstream configFile(Path);
        std::string ils_address;
        std::string ils_port;
        std::string server_port;
        std::vector<std::string> trap_ips;
        // std::cout << "Trying to open the file" << std::endl;
        auto data = json::parse(configFile);

        ils_address = data["Ils"]["IpAddress"];
        ils_port = std::to_string(int(data["Ils"]["Port"]));
        server_port = std::to_string(int(data["Server"]["Port"]));
        trap_ips = data["TrapIps"];
        std::string user_name_encrypted = data["Ils"]["User"]["Name"];
        std::string user_name;
        std::string decrypt = "de -d ";
        std::string caca;
        auto cmd = subprocess::command{decrypt + user_name_encrypted};
        cmd > user_name;
        cmd >= caca;

        try {
            cmd.run();
        } catch (...) {
            user_name = "unknown";
        }
        trim(user_name);

        std::vector<std::vector<std::string>> table_content;
        table_content.push_back({"Parameter", "Value"});
        table_content.push_back({"Ils Address", ils_address});
        table_content.push_back({"Ils Port", ils_port});
        table_content.push_back({"Server Port", server_port});
        table_content.push_back({"User Name", user_name});
        for (const auto ip : trap_ips) {
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
    auto leftButton = Button(action_text, do_action, button_style);
    auto rightButton = Button(" Cancel ", do_cancel, button_style);
    auto component = Container::Vertical({leftButton, rightButton});
    // Polish how the two buttons are rendered
    component |= Renderer([&](Element inner) {
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

Elements Split(std::string the_text)
{
    Elements output;
    std::stringstream ss(the_text);
    std::string word;
    while (std::getline(ss, word, '\n'))
        output.push_back(text(word));
    return output;
}

Element logRender(std::string the_text)
{
    Elements lines;
    for (auto &line : Split(std::move(the_text)))
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
        std::string password_encrypted = data["Server"]["Passwor"];
        std::string decrypt = "de -d ";
        std::string caca;
        auto cmd = subprocess::command{decrypt + password_encrypted};
        cmd > password_plain;
        cmd >= caca;

        try {
            cmd.run();
            trim(password_plain);
        } catch (...) {
            password_encrypted = "nolosabeNadie!";
        }
    } catch (...) {
        password_plain = "nolosabeNadie!";
    }

    int shift = 0;
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
    const std::string &label_login = "Login";

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
    auto input_password = Input(&password_text, "", password_input_option);
    auto password_component = Container::Vertical(
        {Container::Horizontal({input_password}),
         Container::Vertical({login_button, cancel_login_button})});
    auto login_renderer = Renderer(password_component, [&] {
        auto input_win = window(
            text("Login"),
            vbox(
                {hbox({
                     text("Password: "),
                     input_password->Render(),
                 }) | size(WIDTH, EQUAL, 20)
                     | size(HEIGHT, EQUAL, 1),
                 filler(),
                 separator(),
                 hbox({
                     login_button->Render(),
                     separatorEmpty(),
                     cancel_login_button->Render(),
                 })}));
        return vbox({
                   hbox({
                       input_win,
                       filler(),
                   }) | size(HEIGHT, LESS_THAN, 8),
               })
               | flex_grow;
    });
    // ---------------------------------------------------------------------------
    // Compiler
    // ---------------------------------------------------------------------------

    const std::vector<std::string> compiler_entries = {
        "gcc",
        "clang",
        "emcc",
        "game_maker",
        "Ada compilers",
        "ALGOL 60 compilers",
        "ALGOL 68 compilers",
        "Assemblers (Intel *86)",
        "Assemblers (Motorola 68*)",
        "Assemblers (Zilog Z80)",
        "Assemblers (other)",
        "BASIC Compilers",
        "BASIC interpreters",
        "Batch compilers",
        "C compilers",
        "Source-to-source compilers",
        "C++ compilers",
        "C# compilers",
        "COBOL compilers",
        "Common Lisp compilers",
        "D compilers",
        "DIBOL/DBL compilers",
        "ECMAScript interpreters",
        "Eiffel compilers",
        "Fortran compilers",
        "Go compilers",
        "Haskell compilers",
        "Java compilers",
        "Pascal compilers",
        "Perl Interpreters",
        "PHP compilers",
        "PL/I compilers",
        "Python compilers",
        "Scheme compilers and interpreters",
        "Smalltalk compilers",
        "Tcl Interpreters",
        "VMS Interpreters",
        "Rexx Interpreters",
        "CLI compilers",
    };

    int compiler_selected = 0;
    Component compiler = Radiobox(&compiler_entries, &compiler_selected);

    std::array<std::string, 8> options_label = {
        "-Wall",
        "-Werror",
        "-lpthread",
        "-O3",
        "-Wabi-tag",
        "-Wno-class-conversion",
        "-Wcomma-subscript",
        "-Wno-conversion-null",
    };
    std::array<bool, 8> options_state = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
    };

    std::vector<std::string> input_entries;
    int input_selected = 0;
    Component input = Menu(&input_entries, &input_selected);

    auto input_option = InputOption();
    std::string input_add_content;
    input_option.on_enter = [&] {
        input_entries.push_back(input_add_content);
        input_add_content = "";
    };
    Component input_add = Input(&input_add_content, "input files", input_option);

    std::string executable_content_;
    Component executable_ = Input(&executable_content_, "executable");

    Component flags = Container::Vertical({
        Checkbox(options_label.data(), options_state.data()),
        Checkbox(&options_label[1], &options_state[1]),
        Checkbox(&options_label[2], &options_state[2]),
        Checkbox(&options_label[3], &options_state[3]),
        Checkbox(&options_label[4], &options_state[4]),
        Checkbox(&options_label[5], &options_state[5]),
        Checkbox(&options_label[6], &options_state[6]),
        Checkbox(&options_label[7], &options_state[7]),
    });

    auto compiler_component = Container::Horizontal({
        compiler,
        flags,
        Container::Vertical({
            executable_,
            Container::Horizontal({
                input_add,
                input,
            }),
        }),
    });

    auto render_command = [&] {
        Elements line;
        // Compiler
        line.push_back(text(compiler_entries[compiler_selected]) | bold);
        // flags
        for (int i = 0; i < 8; ++i) {
            if (options_state[i]) {
                line.push_back(text(" "));
                line.push_back(text(options_label[i]) | dim);
            }
        }
        // Executable
        if (!executable_content_.empty()) {
            line.push_back(text(" -o ") | bold);
            line.push_back(text(executable_content_) | color(Color::BlueLight) | bold);
        }
        // Input
        for (auto &it : input_entries) {
            line.push_back(text(" " + it) | color(Color::RedLight));
        }
        return line;
    };

    auto compiler_renderer = Renderer(compiler_component, [&] {
        auto compiler_win = window(text("Compiler"), compiler->Render() | vscroll_indicator | frame);
        auto flags_win = window(text("Flags"), flags->Render() | vscroll_indicator | frame);
        auto executable_win = window(text("Executable:"), executable_->Render());
        auto input_win = window(
            text("Input"),
            hbox({
                vbox({
                    hbox({
                        text("Add: "),
                        input_add->Render(),
                    }) | size(WIDTH, EQUAL, 20)
                        | size(HEIGHT, EQUAL, 1),
                    filler(),
                }),
                separator(),
                input->Render() | vscroll_indicator | frame | size(HEIGHT, EQUAL, 3) | flex,
            }));
        return vbox({
                   hbox({
                       compiler_win,
                       flags_win,
                       vbox({
                           executable_win | size(WIDTH, EQUAL, 20),
                           input_win | size(WIDTH, EQUAL, 60),
                       }),
                       filler(),
                   }) | size(HEIGHT, LESS_THAN, 8),
                   hflow(render_command()) | flex_grow,
               })
               | flex_grow;
    });

    // ---------------------------------------------------------------------------
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

    auto scroller = Scroller(log_renderer);

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
            NetworkConfig(),
            GatewayConfig(),
            compiler_renderer,
            scroller,
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
