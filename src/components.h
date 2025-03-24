//
// Created by gzmorell on 23/03/25.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"  // for MenuOption, InputOption
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/table.hpp"
#include "subprocess.hpp"
#include "elements.h"
#include "scrollerbase.h"
#include "util.h"

using namespace ftxui;

const auto button_style = ButtonOption::Animated();

inline Component DummyWindowContent()
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
            std::string error;

            subprocess::command cmd = subprocess::command("cat /var/log/messages");
            cmd >= error;
            try {
                cmd.run();
            } catch (...) {
                str = "Unable to read log file!\n";
                str.append(error);
            }

            // try {
            //     ((("cat /var/log/messages"_cmd >= "/dev/null") | ("grep rms"_cmd >= "/dev/null")) > str).run();
            // } catch (...) {
            //     str = "No messages";
            // }
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

inline Component ModalComponent(
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
                            separatorEmpty(),
                            inner | xflex_grow,
                            separatorEmpty(),
                        },
                        config)})
               | size(WIDTH, GREATER_THAN, 30) | border;
    });
    return component;
}

inline Component Logger(const Component &child)
{
    return Make<ScrollerBase>(child);
}

inline Component gateway_config()
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
        for (const auto &ip : gw_config.trap_ips) {
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

#endif //COMPONENTS_H
