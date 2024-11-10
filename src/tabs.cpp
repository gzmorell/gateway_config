// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/flexbox_config.hpp>
#include <ftxui/screen/color.hpp>
#include <memory>  // for allocator, __shared_ptr_access, shared_ptr
#include <string>  // for string, basic_string
#include <vector>  // for vector

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Radiobox, Renderer, Tab, Toggle, Vertical
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, separator, operator|, vbox, border

using namespace ftxui;

const auto button_style = ButtonOption::Animated();

Component ModalComponent(std::function<void()> do_action,
                         std::function<void()> do_cancel,
                         const std::string& action_text) {
    auto leftButton = Button(action_text, do_action, button_style);
    auto rightButton = Button(" Cancel ", do_cancel, button_style);
    auto component = Container::Vertical({leftButton, rightButton});
    // Polish how the two buttons are rendered
    component |= Renderer([&](Element inner) {
        FlexboxConfig config;
        config.justify_content = FlexboxConfig::JustifyContent::SpaceBetween;
        return vbox({text("Confirm Action") | center, separator(),
                     flexbox(
                         {
                             inner | xflex_grow,
                         },
                         config)}) |
               size(WIDTH, GREATER_THAN, 30) | border;
    });
    return component;
}

Component DummyWindowContent() {
    class Impl : public ComponentBase {
        private:
            float scroll_x = 0.1;
            float scroll_y = 0.1;

        public:
            Impl() {
                auto content = Renderer([=] {
                    const std::string lorem =
                        "Lorem ipsum dolor sit amet, consectetur adipiscing "
                        "elit, sed "
                        "do eiusmod tempor incididunt ut labore et dolore "
                        "magna "
                        "aliqua. Ut enim ad minim veniam, quis nostrud "
                        "exercitation "
                        "ullamco laboris nisi ut aliquip ex ea commodo "
                        "consequat. Duis "
                        "aute irure dolor in reprehenderit in voluptate velit "
                        "esse "
                        "cillum dolore eu fugiat nulla pariatur. Excepteur "
                        "sint "
                        "occaecat cupidatat non proident, sunt in culpa qui "
                        "officia "
                        "deserunt mollit anim id est laborum.";
                    return vbox({
                        text(lorem.substr(0, -1)),
                        text(lorem.substr(5, -1)),
                        text(""),
                        text(lorem.substr(10, -1)),
                        text(lorem.substr(15, -1)),
                        text(""),
                        text(lorem.substr(20, -1)),
                        text(lorem.substr(25, -1)),
                        text(""),
                        text(lorem.substr(30, -1)),
                        text(lorem.substr(35, -1)),
                        text(""),
                        text(lorem.substr(40, -1)),
                        text(lorem.substr(45, -1)),
                        text(""),
                        text(lorem.substr(50, -1)),
                        text(lorem.substr(55, -1)),
                        text(""),
                        text(lorem.substr(60, -1)),
                        text(lorem.substr(65, -1)),
                        text(""),
                        text(lorem.substr(70, -1)),
                        text(lorem.substr(75, -1)),
                        text(""),
                        text(lorem.substr(80, -1)),
                        text(lorem.substr(85, -1)),
                        text(""),
                        text(lorem.substr(90, -1)),
                        text(lorem.substr(95, -1)),
                        text(""),
                        text(lorem.substr(100, -1)),
                        text(lorem.substr(105, -1)),
                        text(""),
                        text(lorem.substr(110, -1)),
                        text(lorem.substr(115, -1)),
                        text(""),
                        text(lorem.substr(120, -1)),
                        text(lorem.substr(125, -1)),
                        text(""),
                        text(lorem.substr(130, -1)),
                        text(lorem.substr(135, -1)),
                        text(""),
                        text(lorem.substr(140, -1)),
                    });
                });

                auto scrollable_content = Renderer(content, [&, content] {
                    return content->Render() |
                           focusPositionRelative(scroll_x, scroll_y) | frame |
                           flex;
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

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<std::string> tab_values{
        " CONFIG ",
        "  LOG   ",
        "tab_3",
    };
    int tab_selected = 0;
    auto tab_toggle = Toggle(&tab_values, &tab_selected);

    std::vector<std::string> tab_1_entries{
        "Forest",
        "Water",
        "I don't know",
    };
    int tab_1_selected = 0;

    std::vector<std::string> tab_2_entries{
        "Hello",
        "Hi",
        "Hay",
    };
    int tab_2_selected = 0;

    std::vector<std::string> tab_3_entries{
        "Table",
        "Nothing",
        "Is",
        "Empty",
    };
    int tab_3_selected = 0;
    auto tab_container = Container::Tab(
        {
            Radiobox(&tab_1_entries, &tab_1_selected),
            DummyWindowContent(),
            Radiobox(&tab_3_entries, &tab_3_selected),
        },
        &tab_selected);

    // auto container = Container::Vertical({
    //     tab_toggle,
    //     tab_container,
    // });

    bool modal_shown = false;
    auto run_exit = screen.ExitLoopClosure();
    auto do_cancel = [&] { modal_shown = false; };

    auto show_modal = [&] { modal_shown = true; };
    const std::string& action_text = "Reboot";
    auto modal_component = ModalComponent(run_exit, do_cancel, action_text);

    std::string exit_label = "Exit";
    std::string reboot_label = "Reboot";

    auto exit_button =
        Button(&exit_label, screen.ExitLoopClosure(), ButtonOption::Animated());
    auto reboot_button =
        Button(&reboot_label, show_modal, ButtonOption::Animated());

    auto hcontainer = Container::Horizontal({Container::Vertical({
                                                 tab_toggle,
                                                 tab_container,
                                             }),
                                             Container::Vertical({
                                                 exit_button,
                                                 reboot_button,
                                             }

                                                                 )});

    auto renderer = Renderer(hcontainer, [&] {
        return vbox({text("Gateway Config") | bold | hcenter, separatorLight(),
                     hbox({
                         vbox({
                             tab_toggle->Render(),
                             separator(),
                             tab_container->Render(),
                         }) | border |
                             flex,
                         window(text("Actions"),
                                vbox({exit_button->Render(), separatorLight(),
                                      reboot_button->Render()})),
                     }) | flex});
    });
    renderer |= Modal(modal_component, &modal_shown);
    screen.Loop(renderer);
}
