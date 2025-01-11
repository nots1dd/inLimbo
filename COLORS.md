# inLimbo Colors - Color Schema Documentation

This document explains the color schema used in [**inLimbo**](https://github.com/nots1dd/inLimbo), including all available colors, their respective RGB values, and how to configure them in the `config.toml` file.

## Overview

The inLimbo project uses a set of predefined true colors, each mapped to an `ftxui::Color::RGB` value (True Color Palette). This allows for a variety of colors to be used within the application to enhance the user interface.

> [!NOTE]
> 
> **inLimbo uses TRUECOLORS**!!
> 
> inLimbo uses `ftxui::Color::RGB` to avoid any terminal emulator
> color schema messing up the colors 
> 
> For example, if you have `pywal` or `wallust` setup with your terminal,
> the colors would **NOT** work as intended with `ftxui::Color` only
> 

### How to Set Colors in `config.toml`

To set a color in the `$HOME/.config/inLimbo/config.toml` file, use the corresponding color name defined in the color enum. The color name must be set under the appropriate section in the TOML file.

For example, if you want to change the active window color to `Red`, you would include the following in your `config.toml`:

```toml
[colors]
active_win_color = "Red"
```

> [!IMPORTANT]
> 
> The color names are case-sensitive and should match exactly as shown in the available color list below.
> 

### Available Colors and Their RGB Values

Here’s a list of all available colors and their respective RGB values:

| Color Name      | RGB Value       |
|-----------------|-----------------|
| Black           | RGB(0, 0, 0)    |
| White           | RGB(255, 255, 255) |
| Red             | RGB(255, 0, 0)  |
| LightRed        | RGB(255, 102, 102) |
| Green           | RGB(0, 255, 0)  |
| LightGreen      | RGB(144, 238, 144) |
| Blue            | RGB(0, 0, 255)  |
| LightBlue       | RGB(173, 216, 230) |
| Yellow          | RGB(255, 255, 0) |
| LightYellow     | RGB(255, 255, 153) |
| Cyan            | RGB(0, 255, 255) |
| LightCyan       | RGB(224, 255, 255) |
| Magenta         | RGB(255, 0, 255) |
| LightMagenta    | RGB(255, 153, 255) |
| Gray            | RGB(128, 128, 128) |
| LightGray       | RGB(211, 211, 211) |
| DarkGray        | RGB(64, 64, 64) |
| Orange          | RGB(255, 165, 0) |
| LightOrange     | RGB(255, 200, 124) |
| Purple          | RGB(128, 0, 128) |
| LightPurple     | RGB(216, 191, 216) |
| Pink            | RGB(255, 192, 203) |
| LightPink       | RGB(255, 182, 193) |
| Teal            | RGB(0, 128, 128) |
| LightTeal       | RGB(144, 224, 224) |
| SkyBlue         | RGB(135, 206, 235) |
| Coral           | RGB(255, 127, 80) |
| Lime            | RGB(191, 255, 0) |
| Lavender        | RGB(230, 230, 250) |
| Crimson         | RGB(220, 20, 60) |
| Gold            | RGB(255, 215, 0) |
| Indigo          | RGB(75, 0, 130)  |
| Mint            | RGB(152, 255, 152) |
| Navy            | RGB(0, 0, 128)  |
| Peach           | RGB(255, 218, 185) |
| Sand            | RGB(244, 164, 96) |
| SeaGreen        | RGB(46, 139, 87) |
| LightSeaGreen   | RGB(152, 255, 204) |
| SlateBlue       | RGB(106, 90, 205) |
| LightSlateBlue  | RGB(176, 196, 222) |
| SunsetOrange    | RGB(255, 99, 71) |
| Turquoise       | RGB(64, 224, 208) |
| LightTurquoise  | RGB(175, 238, 238) |

### How to Use Colors in the inLimbo

Once the color is defined in the `config.toml` file, the `parseColors` function in the code will map the string to the corresponding `TrueColors::Color` enum value. The color is then used within the inLimbo wherever needed.

For example, setting the `active_win_color` in the `config.toml` like this:

```toml
[colors]
active_win_color = "Blue"
```

This will configure the inLimbo to use the color `Blue` (RGB(0, 0, 255)) for the active window color.

### Example of `config.toml`

```toml
[colors]
active_win_color = "Red"
```

This would set the `active_win_color` to red (RGB(255, 0, 0)).

---

By understanding the available colors and how to configure them in the `config.toml` file, you can easily customize the color scheme of your inLimbo player to suit your preferences.

> [!NOTE]
> 
> Theming is fully up!
> 
> It is quite simple, just put the hexadecimal value in the required field :)
> 
> I will make a few basic themes as an example in the future
> 
