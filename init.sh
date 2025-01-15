#!/bin/bash

# Basic ANSI escape values (color display in terminal)
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
PURPLE="\033[35m"
CYAN="\033[36m"
PINK="\033[95m"
RESET="\033[0m"

# Some URL definitions
TOML_PARSER_URL="https://raw.githubusercontent.com/marzer/tomlplusplus/master/toml.hpp"
MINIAUDIO_URL="https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h"
CIMG_URL="https://raw.githubusercontent.com/GreycLab/CImg/refs/heads/master/CImg.h"

echo -e "${CYAN}------------------ Initializing inLimbo Repository --------------------${RESET}"

print_delim() {
  echo -e "${BLUE}===================================================${RESET}"
}

show_msg() {
  local message=$1
  local url=$2
  local target_dir=$3
  local filename=$(basename "$url")
  
  print_delim
  echo -e "${YELLOW}${message}${RESET}"
  echo -e "${CYAN}Downloading to ${target_dir}...${RESET}"

  # Create the target directory if it doesn't exist
  mkdir -p "$target_dir"

  # Download the file to the specified directory
  if curl -o "${target_dir}/${filename}" "$url"; then
    echo -e "${GREEN}Successfully downloaded ${filename} to ${target_dir}.${RESET}"
  else
    echo -e "${RED}Failed to download ${filename}.${RESET}"
    exit 1
  fi

  print_delim
}

# Example usage
show_msg "1. Download header 'toml.hpp'" $TOML_PARSER_URL "./src/parser/"
show_msg "2. Download header 'miniaudio.h'" $MINIAUDIO_URL "./src/music/"
show_msg "3. Download header 'CImg.h'" $CIMG_URL "./src/ui/components/libs/"

echo -e "${CYAN}------------------ Initialization DONE --------------------${RESET}"
