#ifndef SCROLLER_H
#define SCROLLER_H

#include <algorithm>                           // for max, min
#include <ftxui/component/component_base.hpp>  // for Component, ComponentBase
#include <ftxui/component/event.hpp>  // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include <memory>   // for shared_ptr, allocator, __shared_ptr_access
#include <utility>  // for move

#include <ftxui/component/component.hpp>  // for Make
#include <ftxui/component/mouse.hpp>  // for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include <ftxui/dom/deprecated.hpp>  // for text
#include <ftxui/dom/elements.hpp>  // for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include <ftxui/dom/node.hpp>      // for Node
#include <ftxui/dom/requirement.hpp>  // for Requirement
#include <ftxui/screen/box.hpp>       // for Box

namespace ftxui {
  Component Scroller(Component child);
}

#endif
