// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
// #include <stddef.h>    // for size_t
#include <array>  // for array
#include <atomic> // for atomic
// #include <chrono>      // for operator""s, chrono_literals
// #include <cmath>       // for sin
#include <functional> // for ref, reference_wrapper, function
// #include <memory>      // for allocator, shared_ptr, __shared_ptr_access
#include <sstream>
#include <string> // for string, basic_string, char_traits, operator+, to_string
#include <string>
#include <thread>  // for sleep_for, thread
#include <utility> // for move
#include <vector>  // for vector
#include <fstream>
#include <nlohmann/json.hpp>

// #include "../dom/color_info_sorted_2d.ipp"  // for ColorInfoSorted2D
#include "ftxui/component/component.hpp" // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase, Component
#include "ftxui/component/component_options.hpp"  // for MenuOption, InputOption
#include "ftxui/component/event.hpp"              // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
// #include "ftxui/dom/flexbox_config.hpp"  // for FlexboxConfig
#include "ftxui/screen/color.hpp" // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
// #include "ftxui/screen/color_info.hpp"  // for ColorInfo
// #include "ftxui/screen/terminal.hpp"    // for Size, Dimensions
#include "subprocess.hpp"

using namespace ftxui;

const auto button_style = ButtonOption::Animated();

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

Element nonWrappingParagraph(std::string the_text)
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
            auto content = Renderer([=] {
                const std::string str =
                    R"##(
Oct 31 00:09:39 cm4 user.info : rmsMain: Gateway version:  "0.0.4"
Oct 31 00:09:39 cm4 user.info : rmsMain: Command output:  0000000000000000000000000000000000000000000000000000000000000000
Oct 31 00:09:39 cm4 user.info : rmsMain: Command output raw:  "0000000000000000000000000000000000000000000000000000000000000000"
Oct 31 00:09:39 cm4 user.info : rmsMain: Command output size:  64
Oct 31 00:09:39 cm4 user.info : rms.Settings: Settings loaded
Oct 31 00:09:39 cm4 user.info : rms.Settings: Ils Address:  QHostAddress("192.168.1.156")
Oct 31 00:09:39 cm4 user.info : rms.Settings: User:  "GATEWAY"  Password:  "GATEWAY1 GATEWAY2 GATEWAY3"
Oct 31 00:09:39 cm4 user.info : rms.Parameters: Parameters definitions loaded
Oct 31 00:09:39 cm4 user.info : rms.Commands: Commands definitions loaded
Oct 31 00:09:39 cm4 user.info : rms.Engine: Engine Start
Oct 31 00:09:39 cm4 user.info : rms.Engine: Modbus server started
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "rmmEngine"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "rmmDisconnected"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "ilsEngine"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "ilsDisconnected"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "ilsUnlinked"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "ilsLinked"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "logoutState"
Oct 31 00:09:39 cm4 user.info : rms.Engine: onEntry:  "logonState"
Oct 31 00:09:40 cm4 user.info : rms.Engine: onEntry:  "loggedState"
Oct 31 00:09:40 cm4 user.info : rms.Engine: onEntry:  "setupState"
Oct 31 12:41:50 cm4 user.info : rms.Engine: Stopping Engine
Oct 31 12:41:50 cm4 user.info : rms.Engine: Engine Stopped
Nov  7 02:29:35 cm4 user.info : rmsMain: Gateway version:  "0.0.4"
Nov  7 02:29:35 cm4 user.info : rmsMain: Command output:  0000000000000000000000000000000000000000000000000000000000000000
Nov  7 02:29:35 cm4 user.info : rmsMain: Command output raw:  "0000000000000000000000000000000000000000000000000000000000000000"
Nov  7 02:29:35 cm4 user.info : rmsMain: Command output size:  64
Nov  7 02:29:35 cm4 user.info : rms.Settings: Settings loaded
Nov  7 02:29:35 cm4 user.info : rms.Settings: Ils Address:  QHostAddress("192.168.1.156")
Nov  7 02:29:35 cm4 user.info : rms.Settings: User:  "GATEWAY"  Password:  "GATEWAY1 GATEWAY2 GATEWAY3"
Nov  7 02:29:35 cm4 user.info : rms.Parameters: Parameters definitions loaded
Nov  7 02:29:35 cm4 user.info : rms.Commands: Commands definitions loaded
Nov  7 02:29:35 cm4 user.info : rms.Engine: Engine Start
Nov  7 02:29:35 cm4 user.info : rms.Engine: Modbus server started
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "rmmEngine"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "rmmDisconnected"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "ilsEngine"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "ilsDisconnected"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "ilsUnlinked"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "ilsLinked"
Nov  7 02:29:35 cm4 user.info : rms.Engine: onEntry:  "logoutState"
Nov  7 02:29:36 cm4 user.info : rms.Engine: onEntry:  "logonState"
Nov  7 02:29:36 cm4 user.info : rms.Engine: onEntry:  "loggedState"
Nov  7 02:29:36 cm4 user.info : rms.Engine: onEntry:  "setupState"
Nov  7 02:29:42 cm4 user.info : rms.Engine: Stopping Engine
Nov  7 02:29:42 cm4 user.info : rms.Engine: Engine Stopped
Nov  7 15:55:06 cm4 user.info : rmsMain: Gateway version:  "0.0.4"
Nov  7 15:55:06 cm4 user.info : rmsMain: Command output:  0000000000000000000000000000000000000000000000000000000000000000
Nov  7 15:55:06 cm4 user.info : rmsMain: Command output raw:  "0000000000000000000000000000000000000000000000000000000000000000"
Nov  7 15:55:06 cm4 user.info : rmsMain: Command output size:  64
Nov  7 15:55:06 cm4 user.info : rms.Settings: Settings loaded
Nov  7 15:55:06 cm4 user.info : rms.Settings: Ils Address:  QHostAddress("192.168.1.156")
Nov  7 15:55:06 cm4 user.info : rms.Settings: User:  "GATEWAY"  Password:  "GATEWAY1 GATEWAY2 GATEWAY3"
Nov  7 15:55:07 cm4 user.info : rms.Parameters: Parameters definitions loaded
Nov  7 15:55:07 cm4 user.info : rms.Commands: Commands definitions loaded
Nov  7 15:55:07 cm4 user.info : rms.Engine: Engine Start
Nov  7 15:55:07 cm4 user.info : rms.Engine: Modbus server started
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "rmmEngine"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "rmmDisconnected"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "ilsEngine"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "ilsDisconnected"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "ilsUnlinked"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "ilsLinked"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "logoutState"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "logonState"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "loggedState"
Nov  7 15:55:07 cm4 user.info : rms.Engine: onEntry:  "setupState"
Nov  7 15:56:49 cm4 user.info : rms.Engine: Stopping Engine
Nov  7 15:56:49 cm4 user.info : rms.Engine: Engine Stopped
)##";

                return nonWrappingParagraph(str);
            });

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
        }
    };
    return Make<Impl>();
}

int main()
{
    auto screen = ScreenInteractive::Fullscreen();

    int shift = 0;
    bool shown_exit = false;
    bool shown_reboot = false;
    bool shown_save = false;
    auto run_exit = [&] {screen.Exit();};
    auto run_reboot = [&] {
        subprocess::command cmd{"sudo /sbin/reboot"};
        try {
            cmd.run();
        } catch(...) {
            return;
        }
        shown_save = false;
    };
    auto run_save= [&] {
        subprocess::command cmd{"sudo /sbin/lbu ci"};
        try {
            cmd.run();
        } catch(...) {
            return;
        }
        shown_save = false;
    };
    auto cancel_exit = [&] { shown_exit = false; };
    auto show_exit = [&] { shown_exit = true; };
    auto cancel_reboot = [&] { shown_reboot = false; };
    auto show_reboot = [&] { shown_reboot = true; };
    auto cancel_save = [&] { shown_save = false; };
    auto show_save= [&] { shown_save = true; };
    const std::string &label_exit = "Exit";
    const std::string &label_reboot = "Reboot";
    const std::string &label_save = "Save";
    auto component_exit = ModalComponent(run_exit, cancel_exit, label_exit);
    auto component_reboot = ModalComponent(run_reboot, cancel_reboot, label_reboot);
    auto component_save= ModalComponent(run_save, cancel_save, label_save);

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

    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        "Config",
        "Logs",
    };
    auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
    auto tab_content = Container::Tab(
        {
            compiler_renderer,
            DummyWindowContent(),
        },
        &tab_index);

    auto button_exit = Button(label_exit, show_exit, ButtonOption::Animated());
    auto button_reboot = Button(label_reboot, show_reboot, ButtonOption::Animated());
    auto button_save= Button(label_save, show_save, ButtonOption::Animated());

    auto main_container = Container::Vertical({
        Container::Horizontal({
            tab_selection,
            button_save,
            button_reboot,
            button_exit,
        }),
        tab_content,
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
    main_renderer |= Modal(component_save, &shown_save);
    main_renderer |= Modal(component_reboot, &shown_reboot);
    main_renderer |= Modal(component_exit, &shown_exit);

    screen.Loop(main_renderer);

    return 0;
}
