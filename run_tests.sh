#!/bin/bash

# Test Suite script for the inLimbo project

# static decl ik will fix it
declare -A test_binaries
test_binaries=(
  ["tests/audio"]="MiniAudioPlayerTests"
  ["tests/taglib"]="TagLibParserTests"
  ["tests/toml"]="TomlParserTests"
)

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

total_tests=0
passed_tests=0
failed_tests=0
start_time=$(date +%s)

run_test() {
  local test_dir=$1
  local test_bin_name=$2
  local test_binary="build/$test_bin_name"
  
  echo -e "\n${CYAN}${BOLD}==> Running tests in $test_dir with binary: $test_binary${RESET}"

  if [ -f "$test_binary" ]; then
    ((total_tests++))
    echo -e "${YELLOW}  Executing ${BOLD}$test_binary${RESET}..."
    "$test_binary"
    local result=$?
    
    if [ $result -ne 0 ]; then
      ((failed_tests++))
      echo -e "${RED} ** Test in $test_dir failed with exit code $result.**${RESET}"
    else
      ((passed_tests++))
      echo -e "${GREEN}  Test in $test_dir passed successfully!${RESET}"
    fi
  else
    echo -e "${RED}  No test binary found in $test_dir${RESET}"
  fi
}

main() {
  echo -e "${CYAN}${BOLD}==> Running all tests...${RESET}"

  for test_dir in "${!test_binaries[@]}"; do
    test_bin_name="${test_binaries[$test_dir]}"
    
    echo -e "\n${CYAN}${BOLD}==> Entering test directory: $test_dir${RESET}"
    
    cd "$test_dir"
    run_test "$test_dir" "$test_bin_name"
    cd "../.."
  done

  end_time=$(date +%s)
  duration=$((end_time - start_time))

  echo -e "\n${CYAN}${BOLD}==> Test Summary:${RESET}"
  echo -e "${YELLOW}  Total Tests: ${BOLD}$total_tests${RESET}"
  echo -e "${GREEN}  Passed Tests: ${BOLD}$passed_tests${RESET}"
  echo -e "${RED}  Failed Tests: ${BOLD}$failed_tests${RESET}"
  echo -e "${CYAN}  Time Taken: ${BOLD}$duration seconds${RESET}"
  
  if [ $failed_tests -eq 0 ]; then
    echo -e "\n${GREEN}${BOLD}All tests passed successfully!${RESET}"
  else
    echo -e "\n${RED}${BOLD}Some tests failed. Please check the output above.${RESET}"
    return 1
  fi
}

main
