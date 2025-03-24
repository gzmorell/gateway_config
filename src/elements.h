//
// Created by gzmorell on 23/03/25.
//

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <sstream>
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN

using namespace ftxui;

inline Elements Split(const std::string &the_text)
{
    Elements output;
    std::stringstream ss(the_text);
    std::string word;
    while (std::getline(ss, word, '\n'))
        output.push_back(text(word));
    return output;
}

inline Element logRender(const std::string &the_text)
{
    Elements lines;
    for (auto &line : Split(the_text))
        lines.push_back(line);
    return vbox(std::move(lines));
}

#endif //ELEMENTS_H
