// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string> // for string, basic_string, char_traits, operator+, to_string
#include <vector>  // for vector
#include <iostream>

#include "ftxui/component/component.hpp" // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase, Component
#include "ftxui/component/component_options.hpp"  // for MenuOption, InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "components.h"
#include "elements.h"
#include "gatewaysetting.h"
#include "networksetting.h"
#include "rcsusetting.h"
#include "subprocess.hpp"
#include "util.h"

using namespace ftxui;
using namespace subprocess::literals;
using json = nlohmann::json;
namespace fs = std::filesystem;

// const auto button_style = ButtonOption::Animated();
auto log_renderer = Renderer([] {
    std::string str;
    try {
        (("cat /var/log/messages"_cmd >= std::filesystem::path("/dev/null") | "grep rms"_cmd > str)
         >= std::filesystem::path("/dev/null"))
            .run();
    } catch (...) {
        str = "No messages";
    }
    return logRender(str);
});

auto logger = Logger(log_renderer);

int main()
{
    auto screen = ScreenInteractive::Fullscreen();
    std::string password_text;
    int password_count = 0;
    auto password_plain = getPassword();
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
        subprocess::command cmd{"sudo lbu ci"};
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
    const std::string &label_save = "Store";
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

    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        "Network",
        "Gateway",
        "Rcsu",
        "Logs",
    };
    auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
    NetworkSetting network;
    GatewaySetting gateway;
    RcsuSetting rcsu;
    auto tab_content = Container::Tab(
        {
            network.Render(),
            gateway.Render(),
            rcsu.Render(),
            logger,
        },
        &tab_index);

    auto button_exit = Button(label_exit, show_exit, ButtonOption::Animated());
    auto button_reboot = Button(label_reboot, show_reboot, ButtonOption::Animated());
    auto button_save = Button(label_save, show_save, ButtonOption::Animated());
    auto button_login = Button(label_login, show_login, ButtonOption::Animated());

    auto login_container = Container::Horizontal({
        button_login,
    });
    auto main_container = Container::Vertical({
        Container::Horizontal({
            tab_selection,
            button_save,
            button_reboot,
            button_exit
        }),
        tab_content,
    });

    auto logout_renderer = Renderer(login_container, [&] {
        return vbox({
            text("Gateway Login") | bold | hcenter,
            hbox({
                separatorEmpty() | flex,
                button_login->Render(),
                separatorEmpty(),
            }),
        });
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
    logout_renderer = logout_renderer |= Modal(login_renderer, &shown_login);
    logout_renderer |= Modal(component_exit, &shown_exit);
    main_renderer |= Modal(component_save, &shown_save);
    main_renderer |= Modal(component_reboot, &shown_reboot);
    main_renderer |= Modal(component_exit, &shown_exit);

    screen.Loop(logout_renderer);
    screen.Clear();
    if (system_logged)
        screen.Loop(main_renderer);
    std::cout << "Exiting... " << gateway.erased << std::endl;
    return 0;
}
