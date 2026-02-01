#pragma once

#include "InLimbo-Types.hpp"

// basic ASCII char helpers and decls
//
// note that complex xterm compatible keycodes
// like function keys are not supported here.
//
// Most configs and situations in a terminal
// will not require them anyway.

namespace utils::ascii
{

// ------------------------------------------------------------
// Control characters (0..31) + DEL(127)
// ------------------------------------------------------------

static constexpr ui8 NUL = 0; // '\0'
static constexpr ui8 SOH = 1;
static constexpr ui8 STX = 2;
static constexpr ui8 ETX = 3;
static constexpr ui8 EOT = 4;
static constexpr ui8 ENQ = 5;
static constexpr ui8 ACK = 6;
static constexpr ui8 BEL = 7;  // '\a'
static constexpr ui8 BS  = 8;  // backspace -> '\b'
static constexpr ui8 TAB = 9;  // tab space -> '\t'
static constexpr ui8 LF  = 10; // line feed -> '\n'
static constexpr ui8 VT  = 11; // vertical tab -> '\v'
static constexpr ui8 FF  = 12; // form feed -> '\f'
static constexpr ui8 CR  = 13; // carriage return -> '\r'
static constexpr ui8 SO  = 14;
static constexpr ui8 SI  = 15;

static constexpr ui8 DLE = 16;
static constexpr ui8 DC1 = 17;
static constexpr ui8 DC2 = 18;
static constexpr ui8 DC3 = 19;
static constexpr ui8 DC4 = 20;
static constexpr ui8 NAK = 21;
static constexpr ui8 SYN = 22;
static constexpr ui8 ETB = 23;
static constexpr ui8 CAN = 24;
static constexpr ui8 EM  = 25;
static constexpr ui8 SUB = 26;

static constexpr ui8 ESC = 27; // Escape
static constexpr ui8 FS  = 28;
static constexpr ui8 GS  = 29;
static constexpr ui8 RS  = 30;
static constexpr ui8 US  = 31;

// ------------------------------------------------------------
// Common visible/symbol keys (32..126)
// ------------------------------------------------------------

static constexpr ui8 SPACE = 32; // Space char (' ')

static constexpr ui8 EXCLAMATION  = 33; // !
static constexpr ui8 DOUBLE_QUOTE = 34; // "
static constexpr ui8 HASH         = 35; // #
static constexpr ui8 DOLLAR       = 36; // $
static constexpr ui8 PERCENT      = 37; // %
static constexpr ui8 AMPERSAND    = 38; // &
static constexpr ui8 SINGLE_QUOTE = 39; // '
static constexpr ui8 LPAREN       = 40; // (
static constexpr ui8 RPAREN       = 41; // )
static constexpr ui8 ASTERISK     = 42; // *
static constexpr ui8 PLUS         = 43; // +
static constexpr ui8 COMMA        = 44; // ,
static constexpr ui8 MINUS        = 45; // -
static constexpr ui8 DOT          = 46; // .
static constexpr ui8 SLASH        = 47; // /

static constexpr ui8 ZERO  = 48; // 0
static constexpr ui8 ONE   = 49; // 1
static constexpr ui8 TWO   = 50; // 2
static constexpr ui8 THREE = 51; // 3
static constexpr ui8 FOUR  = 52; // 4
static constexpr ui8 FIVE  = 53; // 5
static constexpr ui8 SIX   = 54; // 6
static constexpr ui8 SEVEN = 55; // 7
static constexpr ui8 EIGHT = 56; // 8
static constexpr ui8 NINE  = 57; // 9

static constexpr ui8 COLON     = 58; // :
static constexpr ui8 SEMICOLON = 59; // ;
static constexpr ui8 LT        = 60; // <
static constexpr ui8 EQ        = 61; // =
static constexpr ui8 GT        = 62; // >
static constexpr ui8 QUESTION  = 63; // ?
static constexpr ui8 AT        = 64; // @

// A..Z
static constexpr ui8 A = 65; // A
static constexpr ui8 B = 66; // B
static constexpr ui8 C = 67; // C
static constexpr ui8 D = 68; // D
static constexpr ui8 E = 69; // E
static constexpr ui8 F = 70; // F
static constexpr ui8 G = 71; // G
static constexpr ui8 H = 72; // H
static constexpr ui8 I = 73; // I
static constexpr ui8 J = 74; // J
static constexpr ui8 K = 75; // K
static constexpr ui8 L = 76; // L
static constexpr ui8 M = 77; // M
static constexpr ui8 N = 78; // N
static constexpr ui8 O = 79; // O
static constexpr ui8 P = 80; // P
static constexpr ui8 Q = 81; // Q
static constexpr ui8 R = 82; // R
static constexpr ui8 S = 83; // S
static constexpr ui8 T = 84; // T
static constexpr ui8 U = 85; // U
static constexpr ui8 V = 86; // V
static constexpr ui8 W = 87; // W
static constexpr ui8 X = 88; // X
static constexpr ui8 Y = 89; // Y
static constexpr ui8 Z = 90; // Z

static constexpr ui8 LBRACKET   = 91; // [
static constexpr ui8 BACKSLASH  = 92; // '\'
static constexpr ui8 RBRACKET   = 93; // ]
static constexpr ui8 CARET      = 94; // ^
static constexpr ui8 UNDERSCORE = 95; // _
static constexpr ui8 BACKTICK   = 96; // `

// a..z
static constexpr ui8 a = 97;  // a
static constexpr ui8 b = 98;  // b
static constexpr ui8 c = 99;  // c
static constexpr ui8 d = 100; // d
static constexpr ui8 e = 101; // e
static constexpr ui8 f = 102; // f
static constexpr ui8 g = 103; // g
static constexpr ui8 h = 104; // h
static constexpr ui8 i = 105; // i
static constexpr ui8 j = 106; // j
static constexpr ui8 k = 107; // k
static constexpr ui8 l = 108; // l
static constexpr ui8 m = 109; // m
static constexpr ui8 n = 110; // n
static constexpr ui8 o = 111; // o
static constexpr ui8 p = 112; // p
static constexpr ui8 q = 113; // q
static constexpr ui8 r = 114; // r
static constexpr ui8 s = 115; // s
static constexpr ui8 t = 116; // t
static constexpr ui8 u = 117; // u
static constexpr ui8 v = 118; // v
static constexpr ui8 w = 119; // w
static constexpr ui8 x = 120; // x
static constexpr ui8 y = 121; // y
static constexpr ui8 z = 122; // z

static constexpr ui8 LBRACE = 123; // {
static constexpr ui8 PIPE   = 124; // |
static constexpr ui8 RBRACE = 125; // }
static constexpr ui8 TILDE  = 126; // ~

static constexpr ui8 DEL = 127; // backspace or delete

// aliases
static constexpr ui8 ENTER = LF; // line feed (\n)

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

[[nodiscard]] constexpr auto is_ascii(int v) noexcept -> bool { return v >= 0 && v <= 127; }

[[nodiscard]] constexpr auto is_control(int v) noexcept -> bool
{
  return is_ascii(v) && (v < SPACE || v == TILDE);
}

[[nodiscard]] constexpr auto isPrintable(int v) noexcept -> bool
{
  // Standard printable ASCII range
  return v >= SPACE && v <= TILDE;
}

[[nodiscard]] constexpr auto isWhitespace(int v) noexcept -> bool
{
  return v == SPACE || v == TAB || v == LF || v == VT || v == FF || v == CR;
}

[[nodiscard]] constexpr auto isSpace(int v) noexcept -> bool { return v == SPACE; }

[[nodiscard]] constexpr auto isEnter(int v) noexcept -> bool { return v == LF || v == CR; }

[[nodiscard]] constexpr auto isDigit(int v) noexcept -> bool { return v >= ZERO && v <= NINE; }

[[nodiscard]] constexpr auto isUpper(int v) noexcept -> bool { return v >= A && v <= Z; }

[[nodiscard]] constexpr auto isLower(int v) noexcept -> bool { return v >= a && v <= z; }

[[nodiscard]] constexpr auto isAlpha(int v) noexcept -> bool { return isUpper(v) || isLower(v); }

[[nodiscard]] constexpr auto isAlnum(int v) noexcept -> bool { return isAlpha(v) || isDigit(v); }

[[nodiscard]] constexpr auto toLower(int v) noexcept -> int
{
  if (isUpper(v))
    return v + 32;
  return v;
}

[[nodiscard]] constexpr auto toUpper(int v) noexcept -> int
{
  if (isLower(v))
    return v - 32;
  return v;
}

} // namespace utils::ascii
