#include <memory>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <random>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// dummy structs for UI concept

struct Song {
    std::string title;
    int duration_seconds;
    std::string album;
    int year;
};

struct Artist {
    std::string name;
    std::vector<Song> songs;
};

class MusicPlayer {
public:
    MusicPlayer() {
        InitializeData();
        CreateComponents();
    }

    void Run() {
        auto screen = ScreenInteractive::Fullscreen();
        
        std::thread refresh_thread([&] {
            while (!should_quit) {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(0.1s);
                
                if (is_playing) {
                    current_position += 0.1;
                    if (current_position >= GetCurrentSongDuration()) {
                        if (repeat_mode == RepeatMode::Single) {
                            current_position = 0;
                        } else {
                            PlayNextSong();
                        }
                    }
                }
                
                screen.PostEvent(Event::Custom);
            }
        });

        refresh_thread.detach();
        screen_ = &screen;
        screen.Loop(renderer);
    }

private:
    enum class RepeatMode { None, Single, All };
    enum class ViewMode { Library, Playlist, Search };
    
    std::vector<Artist> library;
    std::vector<std::string> current_artist_names;
    std::vector<std::string> current_song_names;
    std::vector<std::string> playlist;
    
    int selected_artist = 0;
    int selected_song = 0;
    bool is_playing = false;
    bool is_shuffle = false;
    RepeatMode repeat_mode = RepeatMode::None;
    ViewMode current_view = ViewMode::Library;
    
    int volume = 50;
    double current_position = 0;
    std::string search_query;
    bool show_help = false;
    
    bool should_quit = false;
    bool focus_on_artists = true;
    ScreenInteractive* screen_ = nullptr;

    Component artists_list;
    Component songs_list;
    Component controls;
    Component search_input;
    Component renderer;

    const std::vector<std::string> spinner_frames = {
        "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
    };
    int spinner_frame = 0;

    void InitializeData() {
        // Enhanced sample data with more metadata
        library = {
            {"The Beatles", {
                {"Hey Jude", 431, "The Beatles (White Album)", 1968},
                {"Let It Be", 243, "Let It Be", 1970},
                {"Yesterday", 125, "Help!", 1965},
                {"Come Together", 259, "Abbey Road", 1969},
                {"Here Comes the Sun", 185, "Abbey Road", 1969}
            }},
            {"Queen", {
                {"Bohemian Rhapsody", 354, "A Night at the Opera", 1975},
                {"We Will Rock You", 122, "News of the World", 1977},
                {"Don't Stop Me Now", 209, "Jazz", 1978},
                {"Killer Queen", 180, "Sheer Heart Attack", 1974},
                {"Under Pressure", 248, "Hot Space", 1982}
            }},
            {"Pink Floyd", {
                {"Another Brick in the Wall", 205, "The Wall", 1979},
                {"Wish You Were Here", 334, "Wish You Were Here", 1975},
                {"Time", 421, "The Dark Side of the Moon", 1973},
                {"Money", 382, "The Dark Side of the Moon", 1973},
                {"Comfortably Numb", 409, "The Wall", 1979}
            }}
        };

        UpdateCurrentLists();
    }

    void UpdateCurrentLists() {
        current_artist_names.clear();
        for (const auto& artist : library) {
            current_artist_names.push_back(artist.name);
        }

        if (!current_artist_names.empty()) {
            current_song_names.clear();
            for (const auto& song : library[selected_artist].songs) {
                current_song_names.push_back(song.title);
            }
        }
    }

    int GetCurrentSongDuration() {
        if (!current_song_names.empty()) {
            return library[selected_artist].songs[selected_song].duration_seconds;
        }
        return 0;
    }

    std::string FormatTime(int seconds) {
        int minutes = seconds / 60;
        seconds = seconds % 60;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << minutes << ":"
           << std::setfill('0') << std::setw(2) << seconds;
        return ss.str();
    }

    void PlayNextSong() {
        if (current_song_names.empty()) return;

        if (is_shuffle) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, current_song_names.size() - 1);
            selected_song = dis(gen);
        } else {
            selected_song = (selected_song + 1) % current_song_names.size();
        }
        current_position = 0;
    }

    void PlayPreviousSong() {
        if (current_song_names.empty()) return;
        
        if (current_position > 3.0) {
            current_position = 0;
        } else {
            selected_song = (selected_song - 1 + current_song_names.size()) % current_song_names.size();
            current_position = 0;
        }
    }
    void CreateComponents() {
        // Create lists with custom rendering
        artists_list = Radiobox(&current_artist_names, &selected_artist);
        songs_list = Radiobox(&current_song_names, &selected_song);
        search_input = Input(&search_query, "Search...");

        auto main_container = Container::Vertical({
            artists_list,
            songs_list,
            search_input,
        });

        main_container |= CatchEvent([&](Event event) {
            if (event.is_mouse()) return false;

            // Handle character events
            if (event.is_character()) {
                switch (event.character()[0]) {
                    case 'q': case 'Q': Quit(); return true;
                    case ' ': TogglePlayback(); return true;
                    case 'n': PlayNextSong(); return true;
                    case 'p': PlayPreviousSong(); return true;
                    case 'r': CycleRepeatMode(); return true;
                    case 's': is_shuffle = !is_shuffle; return true;
                    case '=': volume = std::min(100, volume + 5); return true;
                    case '-': volume = std::max(0, volume - 5); return true;
                    case 'h': show_help = !show_help; return true;
                    case 'v': current_view = ViewMode::Library; return true;
                    case 'l': current_view = ViewMode::Playlist; return true;
                    case '/': current_view = ViewMode::Search; return true;
                    case 'j': NavigateList(true); return true;
                    case 'k': NavigateList(false); return true;
                }
            }

            // Handle special keys
            if (event == Event::Tab) {
                focus_on_artists = !focus_on_artists;
                return true;
            }
            if (event == Event::ArrowDown) {
                NavigateList(true);
                return true;
            }
            if (event == Event::ArrowUp) {
                NavigateList(false);
                return true;
            }

            return false;
        });

        renderer = Renderer(main_container, [&] {
            // Update current song lists when needed
            if (current_view == ViewMode::Library) {
                UpdateCurrentLists();
            }

            // Progress bar calculation
            int duration = GetCurrentSongDuration();
            float progress = duration > 0 ? (float)current_position / duration : 0;
            
            // Spinner animation
            if (is_playing) {
                spinner_frame = (spinner_frame + 1) % spinner_frames.size();
            }

            // Create the main layout
            Elements layout;
            
            if (show_help) {
                layout.push_back(RenderHelpScreen());
            } else {
                layout.push_back(RenderMainInterface(progress));
            }

            return vbox(layout);
        });
    }

    Element RenderHelpScreen() {
        return vbox({
            text("Keyboard Controls") | bold | color(Color::Green),
            text("─────────────────") | color(Color::Green),
            vbox({
                text("Space  - Play/Pause"),
                text("n      - Next song"),
                text("p      - Previous song"),
                text("r      - Cycle repeat mode"),
                text("s      - Toggle shuffle"),
                text("=      - Volume up"),
                text("-      - Volume down"),
                text("Tab    - Switch focus"),
                text("h      - Toggle help"),
                text("v      - Library view"),
                text("l      - Playlist view"),
                text("/      - Search view"),
                text("q      - Quit"),
            }) | color(Color::Green),
            text(""),
            text("Press 'h' to return to player") | color(Color::Yellow),
        }) | border | flex;
    }

    Element RenderMainInterface(float progress) {
        // Current song info
        std::string current_song_info;
        std::string album_info;
        if (!current_song_names.empty()) {
            const auto& current_artist = library[selected_artist];
            const auto& current_song = current_artist.songs[selected_song];
            current_song_info = current_artist.name + " - " + current_song.title;
            album_info = current_song.album + " (" + std::to_string(current_song.year) + ")";
        }

        // Status indicators
        std::string status = std::string("【")
            + (is_playing ? "▶" : "⏸") + " "
            + (is_shuffle ? "🔀" : "➡") + " "
            + (repeat_mode == RepeatMode::None ? "↩" :
               repeat_mode == RepeatMode::Single ? "🔂" : "🔁")
            + "】";

        // Create the layout
        auto left_pane = vbox({
            text("Library") | bold | color(Color::Green),
            separator(),
            artists_list->Render() | frame | flex
        }) | size(WIDTH, EQUAL, 30) | border;

        auto right_pane = vbox({
            text("Songs") | bold | color(Color::Green),
            separator(),
            songs_list->Render() | frame | flex
        }) | flex | border;

        // Progress bar with time
        auto progress_style = is_playing ? color(Color::Green) : color(Color::GrayDark);
        auto progress_bar = hbox({
            text(FormatTime((int)current_position)) | progress_style,
            gauge(progress) | flex | progress_style,
            text(FormatTime(GetCurrentSongDuration())) | progress_style,
        });

        // Volume indicator
        auto volume_bar = hbox({
            text("Vol:") | dim,
            gauge(volume / 100.0) | size(WIDTH, EQUAL, 10) | color(Color::Yellow),
            text(std::to_string(volume) + "%") | dim,
        });

        // Status bar
        auto status_bar = hbox({
            text(spinner_frames[spinner_frame]) | color(Color::Black),
            text(" "),
            text(status) | color(Color::Black),
            text(" "),
            text(current_song_info) | bold | color(Color::Red),
            text(" - ") | dim,
            text(album_info) | color(Color::Blue),
        }) | size(HEIGHT, EQUAL, 1) | bgcolor(Color::Yellow);

        return vbox({
            hbox({
                left_pane,
                right_pane,
            }) | flex,
            separator(),
            progress_bar,
            hbox({
                volume_bar | flex,
                text(current_view == ViewMode::Library ? "Library" :
                     current_view == ViewMode::Playlist ? "Playlist" : "Search")
                | dim | align_right,
            }),
            status_bar,
        });
    }

    void NavigateList(bool move_down) {
        if (focus_on_artists) {
            if (move_down) {
                selected_artist = (selected_artist + 1) % current_artist_names.size();
            } else {
                selected_artist = (selected_artist - 1 + current_artist_names.size()) % current_artist_names.size();
            }
            selected_song = 0;
        } else {
            if (!current_song_names.empty()) {
                if (move_down) {
                    selected_song = (selected_song + 1) % current_song_names.size();
                } else {
                    selected_song = (selected_song - 1 + current_song_names.size()) % current_song_names.size();
                }
            }
        }
    }

    void TogglePlayback() {
        is_playing = !is_playing;
        if (is_playing && current_song_names.empty()) {
            is_playing = false;
        }
    }

    void CycleRepeatMode() {
        switch (repeat_mode) {
            case RepeatMode::None:
                repeat_mode = RepeatMode::Single;
                break;
            case RepeatMode::Single:
                repeat_mode = RepeatMode::All;
                break;
            case RepeatMode::All:
                repeat_mode = RepeatMode::None;
                break;
        }
    }

    void Quit() {
        should_quit = true;
        if (screen_) {
            screen_->Exit();
        }
    }
};

int main() {
    MusicPlayer player;
    player.Run();
    return 0;
}
