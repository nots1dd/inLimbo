#pragma once

#include <raylib.h>

namespace frontend::raylib::ui
{

static constexpr const char* UI_GLYPHS =
  // Basic Latin
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
  "0123456789"

  // Latin-1 Supplement
  "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß"
  "àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"

  // Latin Extended-A
  "ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚě"
  "ĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİı"
  "ĲĳĴĵĶķĹĺĻļĽľŁłŃńŅņŇňŊŋ"
  "ŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠš"
  "ŢţŤťŦŧŨũŪūŬŭŮůŰűŲų"
  "ŴŵŶŷŸŹźŻżŽž"

  // Punctuation & symbols
  ".,:;!?…"
  "\"'`´“”‘’"
  "()[]{}<>"
  "+-*/=×÷±"
  "%‰&|^~"
  "@#_$"

  // Currency
  "₹€£¥₩₽₿¢"

  // Arrows
  "←↑→↓↔↕⇐⇑⇒⇓"

  // Box drawing
  "─│┌┐└┘├┤┬┴┼"

  // Music / media
  "♪♫♬♭♮♯"
  "⏮⏯⏭⏸▶⏹⏺•"

  // Misc UI
  "✓✔✕✖✗";

struct Fonts
{
  Font regular{};
  Font bold{};

  void load();
  void unload();
};

} // namespace frontend::raylib::ui
