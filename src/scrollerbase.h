//
// Created by gzmorell on 23/03/25.
//

#ifndef SCROLLERBASE_H
#define SCROLLERBASE_H

#include "ftxui/component/component_base.hpp"     // for ComponentBase, Component


namespace ftxui {
class ScrollerBase final : public ComponentBase
{
public:
    explicit ScrollerBase(const Component &child) { Add(child); }

private:
    Element OnRender() override
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

    bool OnEvent(Event event) override
    {
        if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y))
            TakeFocus();

        const int selected_old = selected_;
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

    [[nodiscard]] bool Focusable() const override { return true; }

    int selected_ = 0;
    int size_ = 0;
    Box box_;
};

}

#endif //SCROLLERBASE_H
