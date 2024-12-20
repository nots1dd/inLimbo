#include <memory>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

std::vector<std::string> spinner_frames = {
    "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
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
                screen.PostEvent(Event::Custom);
            }
        });

        refresh_thread.detach();
        
        // Store screen reference for quitting
        screen_ = &screen;
        screen.Loop(renderer);
    }

private:
    std::map<std::string, std::vector<std::string>> artist_songs;
    std::vector<std::string> artists;
    std::vector<std::string> current_songs;
    int selected_artist = 0;
    int selected_song = 0;
    bool is_playing = false;
    int volume = 50;
    int spinner_frame = 0;
    std::string current_song = "";
    bool should_quit = false;
    bool focus_on_artists = true;
    ScreenInteractive* screen_ = nullptr;  // Add screen pointer

    Component artists_list;
    Component songs_list;
    Component play_button;
    Component volume_slider;
    Component renderer;

    void InitializeData() {
        artist_songs = {
            {"The Beatles", {
                "Hey Jude",
                "Let It Be",
                "Yesterday"
            }},
            {"Queen", {
                "Bohemian Rhapsody",
                "We Will Rock You",
                "Don't Stop Me Now"
            }},
            {"Pink Floyd", {
                "Another Brick in the Wall",
                "Wish You Were Here",
                "Time"
            }}
        };

        for (const auto& artist : artist_songs) {
            artists.push_back(artist.first);
        }
        current_songs = artist_songs[artists[0]];
    }

    bool IsValidKey(Event event) {
        return event == Event::Character('q') ||
               event == Event::Character('Q') ||
               event == Event::Tab ||
               event == Event::ArrowLeft ||
               event == Event::Character('h') ||
               event == Event::ArrowRight ||
               event == Event::Character('l') ||
               event == Event::Character(' ') ||
               event == Event::Character('=') ||
               event == Event::Character('-') ||
               event == Event::Character('j') ||
               event == Event::Character('k');
    }

    void NavigateList(bool move_down) {
        if (focus_on_artists) {
            if (move_down) {
                selected_artist = (selected_artist + 1) % artists.size();
            } else {
                selected_artist = (selected_artist - 1 + artists.size()) % artists.size();
            }
            current_songs = artist_songs[artists[selected_artist]];
            selected_song = 0;
        } else {
            if (!current_songs.empty()) {
                if (move_down) {
                    selected_song = (selected_song + 1) % current_songs.size();
                } else {
                    selected_song = (selected_song - 1 + current_songs.size()) % current_songs.size();
                }
            }
        }
    }

    void Quit() {
        should_quit = true;
        if (screen_) {
            screen_->Exit();
        }
    }

    void CreateComponents() {
        artists_list = Radiobox(&artists, &selected_artist);
        songs_list = Radiobox(&current_songs, &selected_song);
        play_button = Button("▶", [&] {
            is_playing = !is_playing;
            if (is_playing && !current_songs.empty()) {
                current_song = current_songs[selected_song];
            }
        });
        volume_slider = Slider("Volume: ", &volume, 0, 100, 1);

        auto main_container = Container::Vertical({
            artists_list,
            songs_list,
            play_button,
            volume_slider,
        });

        main_container |= CatchEvent([&](Event event) {
            if (event.is_mouse()) {
                return false;
            }

            if (!IsValidKey(event)) {
                return true;
            }

            if (event == Event::Character('q') || event == Event::Character('Q')) {
                Quit();
                return true;
            }
            if (event == Event::Tab) {
                focus_on_artists = !focus_on_artists;
                return true;
            }
            if (event == Event::ArrowLeft || event == Event::Character('h')) {
                focus_on_artists = true;
                return true;
            }
            if (event == Event::ArrowRight || event == Event::Character('l')) {
                focus_on_artists = false;
                return true;
            }
            if (event == Event::Character(' ')) {
                is_playing = !is_playing;
                if (is_playing && !current_songs.empty()) {
                    current_song = current_songs[selected_song];
                }
                return true;
            }
            if (event == Event::Character('=')) {
                volume = std::min(100, volume + 5);
                return true;
            }
            if (event == Event::Character('-')) {
                volume = std::max(0, volume - 5);
                return true;
            }
            if (event == Event::Character('j')) {
                NavigateList(true);
                return true;
            }
            if (event == Event::Character('k')) {
                NavigateList(false);
                return true;
            }
            return false;
        });

        renderer = Renderer(main_container, [&] {
            current_songs = artist_songs[artists[selected_artist]];

            if (is_playing) {
                spinner_frame = (spinner_frame + 1) % spinner_frames.size();
            }

            auto status_text = is_playing ? current_song : "Stopped";
            auto status_bar = hbox({
                text(is_playing ? spinner_frames[spinner_frame] : " ") | color(Color::Yellow),
                text(" [") | dim,
                text(std::to_string(volume) + "%") | color(Color::Blue),
                text("] ") | dim,
                text(status_text) | color(Color::Red),
            }) | size(HEIGHT, EQUAL, 1);

            auto left_pane = vbox({
                text("Artists") | bold | color(Color::Green),
                artists_list->Render() | flex
            }) | size(WIDTH, EQUAL, 30) | border;

            if (focus_on_artists) {
                left_pane = left_pane | inverted;
            }

            auto right_pane = vbox({
                text("Songs") | bold | color(Color::Green),
                songs_list->Render() | flex
            }) | flex | border;

            if (!focus_on_artists) {
                right_pane = right_pane | inverted;
            }

            return vbox({
                hbox({
                    left_pane,
                    right_pane,
                }) | flex,
                status_bar | bgcolor(Color::Black),
            });
        });
    }
};

int main() {
    MusicPlayer player;
    player.Run();
    return 0;
}
