#include "./mpris-service.hpp"

int main() {
    MPRISService service("MyMusicPlayer");

    service.updateMetadata(
        "My Song Title",
        "My Artist",
        "My Album",
        180,
        "Some comment",
        "Some genre",
        1, // track
        2 // disc
    );

    // Run the main loop
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    return 0;
}
