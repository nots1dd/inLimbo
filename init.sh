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

declare -A COMPLETION_FILES=(
    ["bash"]="assets/completions/inLimbo-completions.bash"
    ["zsh"]="assets/completions/inLimbo-completions.zsh"
    ["fish"]="assets/completions/inLimbo-completions.fish"
)

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

  if [[ -f "$target_dir/$filename" ]]; then
    echo -e "${GREEN}✓ ${filename} already exists in ${target_dir}, skipping download.${RESET}"
    print_delim
    return 0
  fi

  print_delim
  echo -e "${YELLOW}${message}${RESET}"
  echo -e "${CYAN}Downloading to ${target_dir}...${RESET}"

  # Create the target directory if it doesn't exist
  mkdir -p "$target_dir"

  # Download the file to the specified directory
  if curl -o "${target_dir}/${filename}" "$url"; then
    echo -e "${GREEN}✓ Successfully downloaded ${filename} to ${target_dir}.${RESET}"
  else
    echo -e "${RED}Failed to download ${filename}.${RESET}"
    exit 1
  fi

  print_delim
}

setup_shell_completions() {
    print_delim
    echo -e "${CYAN}Shell Completion Setup${RESET}"
    echo -e "${YELLOW}Which shells would you like to set up completions for?${RESET}"
    echo
    echo -e "${GREEN}Available options:${RESET}"
    echo -e "1) ${BLUE}bash${RESET}"
    echo -e "2) ${BLUE}zsh${RESET}"
    echo -e "3) ${BLUE}fish${RESET}"
    echo -e "4) ${PURPLE}all${RESET}"
    echo -e "0) ${RED}none${RESET}"
    echo
    echo -e "${YELLOW}Enter your choices (space-separated numbers, e.g., '1 2' or '4' for all):${RESET} "
    read -r choices

    # Convert "4" to "1 2 3" (all options)
    if [[ $choices == "4" ]]; then
        choices="1 2 3"
    fi

    # Early exit if none selected
    if [[ $choices == "0" ]]; then
        echo -e "${YELLOW}No shell completions will be set up.${RESET}"
        return 
    fi

    # Process each selected shell
    for choice in $choices; do
        case $choice in
            1)
                setup_bash_completion
                ;;
            2)
                setup_zsh_completion
                ;;
            3)
                setup_fish_completion
                ;;
        esac
    done
}

setup_bash_completion() {
    local bashrc="$HOME/.bashrc"
    local completion_path="$(pwd)/${COMPLETION_FILES[bash]}"
    
    echo -e "${BLUE}Setting up Bash completion...${RESET}"
    if [[ -f "$completion_path" ]]; then
        if ! grep -q "$completion_path" "$bashrc"; then
            echo "source $completion_path" >> "$bashrc"
            echo -e "${GREEN}✓ Bash completion added to $bashrc${RESET}"
        else
            echo -e "${YELLOW}! Bash completion already configured${RESET}"
        fi
    else
        echo -e "${RED}✗ Bash completion file not found at $completion_path${RESET}"
    fi
}

setup_zsh_completion() {
    local zshrc="$HOME/.zshrc"
    local completion_path="$(pwd)/${COMPLETION_FILES[zsh]}"
    
    echo -e "${BLUE}Setting up Zsh completion...${RESET}"
    if [[ -f "$completion_path" ]]; then
        if ! grep -q "$completion_path" "$zshrc"; then
            echo "source $completion_path" >> "$zshrc"
            echo -e "${GREEN}✓ Zsh completion added to $zshrc${RESET}"
        else
            echo -e "${YELLOW}! Zsh completion already configured${RESET}"
        fi
    else
        echo -e "${RED}✗ Zsh completion file not found at $completion_path${RESET}"
    fi
}

setup_fish_completion() {
    local fish_dir="$HOME/.config/fish/completions"
    local completion_path="$(pwd)/${COMPLETION_FILES[fish]}"
    local target_path="$fish_dir/inLimbo.fish"
    
    echo -e "${BLUE}Setting up Fish completion...${RESET}"
    if [[ -f "$completion_path" ]]; then
        mkdir -p "$fish_dir"
        if [[ ! -f "$target_path" ]]; then
            cp "$completion_path" "$target_path"
            echo "source $target_path" >> "$HOME/.config/fish/config.fish"
            echo -e "${GREEN}✓ Fish completion installed to $target_path${RESET}"
        else
            echo -e "${YELLOW}! Fish completion already installed${RESET}"
        fi
    else
        echo -e "${RED}✗ Fish completion file not found at $completion_path${RESET}"
    fi
}

# Example usage
show_msg "1. Download header 'toml.hpp'" $TOML_PARSER_URL "./src/parser/"
show_msg "2. Download header 'miniaudio.h'" $MINIAUDIO_URL "./src/music/"
show_msg "3. Download header 'CImg.h'" $CIMG_URL "./src/ui/components/libs/"

setup_shell_completions

echo -e "${CYAN}------------------ Initialization DONE --------------------${RESET}"
