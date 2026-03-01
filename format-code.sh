#!/bin/bash
# Format all C/C++ source files in the src directory

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Formatting C/C++ files in src/ directory...${NC}"

# Find all .c, .cpp, .h files in src/ and format them
find src/ -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

echo -e "${GREEN}Done!${NC}"
