#pragma once

namespace utils::string
{

/*
  UTF-8 Decoder (single codepoint)

  Reads one UTF-8 encoded character from buffer [p, end)
  Advances pointer p.
  Returns Unicode codepoint (char32_t).

  UTF-8 encoding patterns:

    1 byte  : 0xxxxxxx                      (ASCII)
    2 bytes : 110xxxxx 10xxxxxx
    3 bytes : 1110xxxx 10xxxxxx 10xxxxxx
    4 bytes : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

  Key masks used below:

    0x80 = 10000000  -> ASCII boundary
    0x1F = 00011111  -> Extract 5 payload bits
    0x3F = 00111111  -> Extract 6 payload bits (continuation byte)
    0x0F = 00001111  -> Extract 4 payload bits
    0x07 = 00000111  -> Extract 3 payload bits
*/
static inline auto utf8_decode(const char*& p, const char* end) -> char32_t
{
  // Stop if pointer reached end of buffer
  if (p >= end)
    return 0;

  // Read first byte and advance pointer
  // Cast to unsigned to avoid sign-extension during bit operations
  auto c = (unsigned char)*p++;

  // -----------------------------
  // ASCII fast path
  // -----------------------------
  // ASCII bytes are 0xxxxxxx (0–127)
  // If highest bit is 0 -> single byte character
  if (c < 0x80)
    return c;

  // -----------------------------
  // 2-byte UTF-8 sequence
  // Pattern: 110xxxxx 10xxxxxx
  // -----------------------------
  // c >> 5 extracts top 3 bits
  // 110xxxxx >> 5 = 00000110 (decimal 6 = 0x6)
  if ((c >> 5) == 0x6 && p < end)
  {
    // Remove leading 110 bits → keep 5 payload bits
    // Then shift left 6 because next byte contributes 6 bits
    char32_t r = (c & 0x1F) << 6;

    // Continuation byte format = 10xxxxxx
    // Mask with 0x3F to remove leading 10
    r |= ((unsigned char)*p++ & 0x3F);

    return r;
  }

  // -----------------------------
  // 3-byte UTF-8 sequence
  // Pattern: 1110xxxx 10xxxxxx 10xxxxxx
  // -----------------------------
  // c >> 4 extracts top 4 bits
  // 1110xxxx >> 4 = 00001110 (decimal 14 = 0xE)
  if ((c >> 4) == 0xE && p + 1 < end)
  {
    // Remove leading 1110 bits → keep 4 payload bits
    // Shift left 12 (6 + 6 bits coming from next bytes)
    char32_t r = (c & 0x0F) << 12;

    r |= ((unsigned char)*p++ & 0x3F) << 6;
    r |= ((unsigned char)*p++ & 0x3F);

    return r;
  }

  // -----------------------------
  // 4-byte UTF-8 sequence
  // Pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  // -----------------------------
  // c >> 3 extracts top 5 bits
  // 11110xxx >> 3 = 00011110 (decimal 30 = 0x1E)
  if ((c >> 3) == 0x1E && p + 2 < end)
  {
    // Remove leading 11110 bits → keep 3 payload bits
    // Shift left 18 (6 + 6 + 6 bits coming)
    char32_t r = (c & 0x07) << 18;

    r |= ((unsigned char)*p++ & 0x3F) << 12;
    r |= ((unsigned char)*p++ & 0x3F) << 6;
    r |= ((unsigned char)*p++ & 0x3F);

    return r;
  }

  // Invalid UTF-8 sequence -> return Unicode replacement character
  // U+FFFD = standard "invalid character" symbol
  return 0xFFFD;
}

/*
  Lightweight Unicode lowercase conversion.

  Only handles:
    - ASCII uppercase A-Z
    - Latin-1 uppercase letters (À-Ö and Ø-Þ)

  Does NOT implement full Unicode case folding.
*/
static inline auto unicode_tolower(char32_t c) -> char32_t
{
  // ASCII uppercase → lowercase
  // ASCII layout:
  //   'A' = 65
  //   'a' = 97
  // Difference = +32
  if (c >= U'A' && c <= U'Z')
    return c + 32;

  // Latin-1 uppercase ranges:
  //
  // 0xC0–0xD6 → À to Ö
  // 0xD8–0xDE → Ø to Þ
  //
  // In Latin-1, lowercase is also +32 offset
  //
  // Note: 0xD7 is multiplication sign, so skipped intentionally

  if (c >= 0xC0 && c <= 0xD6)
    return c + 32;

  if (c >= 0xD8 && c <= 0xDE)
    return c + 32;

  // Otherwise return unchanged
  return c;
}

} // namespace utils::string
