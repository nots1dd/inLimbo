#!/bin/bash

# ANSI escape codes for styling
BOLD="\033[1m"
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
RESET="\033[0m"

# Output directory
OUTPUT_DIR="./"
echo -e "${BOLD}${BLUE}Creating output directory: ${OUTPUT_DIR}${RESET}"
mkdir -p "$OUTPUT_DIR"

# Path to album art image (ensure the image file exists)
ALBUM_ART_PATH="./thumbnail.png"

# Function to create an audio file with metadata and embed album art
create_audio_file() {
    local file_name=$1
    local codec=$2
    local title=$3
    local artist=$4
    local album=$5
    local genre=$6
    local year=$7
    local comment=$8
    local track=$9
    local duration=${10}
    local file_name_art=${11}

    # Check if file already exists
    if [ -f "$OUTPUT_DIR/$file_name" ]; then
        echo -e "${BOLD}${YELLOW}$file_name already exists, skipping creation.${RESET}"
        return
    fi

    echo -e "-- ${BOLD}${YELLOW}Generating $file_name with the following metadata:${RESET}"
    echo -e "     ${GREEN}Title:   ${RESET}$title"
    echo -e "     ${GREEN}Artist:  ${RESET}$artist"
    echo -e "     ${GREEN}Album:   ${RESET}$album"
    echo -e "     ${GREEN}Genre:   ${RESET}$genre"
    echo -e "     ${GREEN}Year:    ${RESET}$year"
    echo -e "     ${GREEN}Comment: ${RESET}$comment"
    echo -e "     ${GREEN}Track:   ${RESET}$track"
    echo -e "     ${GREEN}Duration:${RESET} $duration s"

    # Generate a silent audio file with ffmpeg
    ffmpeg -y -f lavfi -i anullsrc=r=44100:cl=stereo -t "$duration" \
        -metadata title="$title" \
        -metadata artist="$artist" \
        -metadata album="$album" \
        -metadata genre="$genre" \
        -metadata date="$year" \
        -metadata comment="$comment" \
        -metadata track="$track" \
        "$OUTPUT_DIR/$file_name" >/dev/null 2>&1

    # Check if the file creation was successful
    if [ $? -eq 0 ]; then
        echo -e "-- ${BOLD}${GREEN}Successfully created $file_name${RESET}"

        # Embed album art into the file
        if [ -f "$ALBUM_ART_PATH" ]; then
            echo -e "-- ${BOLD}${YELLOW}Embedding album art into $file_name_art${RESET}"
            if [[ $file_name == *.mp3 ]]; then
              ffmpeg -i "$OUTPUT_DIR/$file_name" -i "$ALBUM_ART_PATH" -map_metadata 0 -map 0 -map 1 -acodec copy "$OUTPUT_DIR/$file_name_art" >/dev/null 2>&1
            elif [[ $file_name == *.flac ]]; then
                ffmpeg -i "$OUTPUT_DIR/$file_name" -i "$ALBUM_ART_PATH" -map_metadata 0 -map 0 -map 1 -acodec copy -disposition:v attached_pic "$OUTPUT_DIR/$file_name_art" >/dev/null 2>&1
            fi
            echo -e "-- ${BOLD}${GREEN}Album art embedded in $file_name_art${RESET}\n"
        else
            echo -e "-- ${BOLD}${RED}Album art file not found, skipping embedding.${RESET}\n"
        fi
    else
        echo -e "-- ${BOLD}${RED}Failed to create $file_name${RESET}\n"
    fi
}

# Define metadata
TITLE="Test Song"
ARTIST="Test Artist"
ALBUM="Test Album"
GENRE="Test Genre"
YEAR="2025"
COMMENT="This is a test comment."
TRACK="1"
DURATION="10" # 10 seconds

# Create dummy MP3 file
create_audio_file "dummy.mp3" "libmp3lame" "$TITLE" "$ARTIST" "$ALBUM" "$GENRE" "$YEAR" "$COMMENT" "$TRACK" "$DURATION" "dummy_art.mp3"

# Create dummy FLAC file
create_audio_file "dummy.flac" "flac" "$TITLE" "$ARTIST" "$ALBUM" "$GENRE" "$YEAR" "$COMMENT" "$TRACK" "$DURATION" "dummy_art.flac"

# Summary
echo -e "${BOLD}${BLUE}Dummy audio files created in $OUTPUT_DIR ($(pwd))${RESET}"
