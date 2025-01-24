#include "./../../src/music/audio_playback.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>

using namespace std;

#define TEST_SONG_NAME "test_audio.wav"

class MiniAudioPlayerTest : public ::testing::Test {
protected:
    MiniAudioPlayer* player;

    void SetUp() override {
        ASSERT_TRUE(fileExists(TEST_SONG_NAME)) << "Test audio file not found.";
        player = new MiniAudioPlayer();
    }

    void TearDown() override {
        if (player) {
            delete player;
            player = nullptr;
        }
    }

    bool fileExists(const std::string& filename) 
    {
        std::ifstream file(filename);
        return file.good();
    }
};

TEST_F(MiniAudioPlayerTest, InitializePlayer) {
    EXPECT_NO_THROW({
        MiniAudioPlayer testPlayer;
    });
}

TEST_F(MiniAudioPlayerTest, EnumerateAudioDevices) {
    EXPECT_NO_THROW({
        std::cout << "---- Available Audio Devices ----" << std::endl;
        auto devices = player->enumerateDevices();
        EXPECT_GT(devices.size(), 0) << "No audio devices found.";

        for (const auto& device : devices) {
            EXPECT_FALSE(device.name.empty()) << "Device name is empty.";
        }
        std::cout << "---------------------------------" << std::endl;
    });
}

TEST_F(MiniAudioPlayerTest, LoadFileSuccess) {
    std::string testFile = TEST_SONG_NAME;
    ASSERT_NO_THROW({
        EXPECT_EQ(player->loadFile(testFile, false), 0);
    });
}

TEST_F(MiniAudioPlayerTest, LoadInvalidFile) {
    std::string invalidFile = "nonexistent_file.wav";
    EXPECT_THROW({
        player->loadFile(invalidFile, false);
    }, std::runtime_error);
}

TEST_F(MiniAudioPlayerTest, PlaySound) {
    std::string testFile = TEST_SONG_NAME;
    player->loadFile(testFile, false);

    EXPECT_NO_THROW({
        player->play();
    });

    EXPECT_TRUE(player->isCurrentlyPlaying());
}

TEST_F(MiniAudioPlayerTest, PauseAndResume) {
    std::string testFile = TEST_SONG_NAME;
    player->loadFile(testFile, false);
    player->play();

    EXPECT_NO_THROW({
        player->pause();
    });
    EXPECT_FALSE(player->isCurrentlyPlaying());

    EXPECT_NO_THROW({
        player->resume();
    });
    EXPECT_TRUE(player->isCurrentlyPlaying());
}

TEST_F(MiniAudioPlayerTest, PauseBeforePlay) {
    EXPECT_THROW({
        player->pause();
        player->pause();
    }, std::runtime_error);
}

TEST_F(MiniAudioPlayerTest, SetAndGetVolume) {
    float initialVolume = player->getVolume();
    EXPECT_NO_THROW({
        player->setVolume(0.5f);
    });

    EXPECT_FLOAT_EQ(player->getVolume(), 0.5f);

    EXPECT_THROW({
        player->setVolume(-0.1f);
    }, std::invalid_argument);

    EXPECT_THROW({
        player->setVolume(1.5f);
    }, std::invalid_argument);

    EXPECT_NO_THROW({
        player->setVolume(initialVolume);
    });
}

TEST_F(MiniAudioPlayerTest, SeekTime) {
    std::string testFile = TEST_SONG_NAME;
    player->loadFile(testFile, false);
    player->play();

    EXPECT_NO_THROW({
        double newTime = player->seekTime(5);
        EXPECT_GE(newTime, 5.0);
    });

    EXPECT_NO_THROW({
        double newTime = player->seekTime(-5);
        EXPECT_EQ(newTime, 0.0);
    });
}

TEST_F(MiniAudioPlayerTest, InvalidStateTransitions) {
    EXPECT_THROW(player->seekTime(10.0), std::runtime_error); // Seek without loading file
}

TEST_F(MiniAudioPlayerTest, StopPlaying) {
    player->loadFile(TEST_SONG_NAME, false);
    player->play();
    EXPECT_TRUE(player->isCurrentlyPlaying());

    EXPECT_NO_THROW(player->stop());
    EXPECT_FALSE(player->isCurrentlyPlaying());
}

// Test repeated play calls
TEST_F(MiniAudioPlayerTest, MultiplePlayCalls) {
    player->loadFile(TEST_SONG_NAME, false);
    EXPECT_NO_THROW(player->play());
    EXPECT_NO_THROW(player->play());  // Ensure calling play again doesn't break
}

// Test concurrent play and stop
TEST_F(MiniAudioPlayerTest, ConcurrentPlayAndStop) {
    player->loadFile(TEST_SONG_NAME, false);

    std::thread playThread([&]() { player->play(); });
    std::thread stopThread([&]() { player->stop(); });

    playThread.join();
    stopThread.join();

    EXPECT_FALSE(player->isCurrentlyPlaying());
}

TEST_F(MiniAudioPlayerTest, DestructorCleanup) {
    MiniAudioPlayer* tempPlayer = new MiniAudioPlayer();
    tempPlayer->loadFile(TEST_SONG_NAME, false);
    tempPlayer->play();

    EXPECT_NO_THROW(delete tempPlayer);
}

/*TEST_F(MiniAudioPlayerTest, BackendInitialization) {*/
/*    EXPECT_NO_THROW({*/
/*        MiniAudioPlayer testPlayer;*/
/*        EXPECT_TRUE(testPlayer.isBackendInitialized());*/
/*    });*/
/*}*/
