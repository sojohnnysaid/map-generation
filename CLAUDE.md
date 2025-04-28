# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Build: `make all` or just `make`
- Run: `make run`
- Benchmark: `make time`
- Clean: `make clean`

## Code Style Guidelines
- Indentation: 4 spaces
- Line length: ~80 characters 
- Brackets: K&R style (opening bracket on same line)
- Constants: `#define UPPER_SNAKE_CASE`
- Functions/variables: `snake_case`
- Structs: `PascalCase`
- Module prefixes for functions (e.g., `map_` prefix for map operations)

## Error Handling
- Check pointers for NULL before operations
- Use early return pattern with error messages to stderr
- Return 0/NULL for failure, non-zero/object pointer for success
- Clean up allocated resources on error paths

## Memory Management
- Always pair allocations with corresponding deallocations
- Free temporary resources with cleanup functions
- Validate memory allocations immediately after allocation