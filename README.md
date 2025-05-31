# linux-memory-allocator-in-c

A simple custom memory allocator implemented in C, designed for Linux systems. This project demonstrates how basic dynamic memory management (`malloc`, `calloc`, `free`, and `realloc`) can be implemented from scratch using only `sbrk`, with support for thread safety.

## Features

- Custom implementations of `malloc`, `calloc`, `free`, and `realloc`
- Manages memory using the `sbrk` system call
- Safely supports multiple threads using `pthread` locks
- On deallocation: if a freed block is at the end of the heap, the heap is "shrunk" with `sbrk` to release memory back to the OS
- Clean, well-commented, beginner-friendly C code
- Educational: learn how memory management works under the hood

## Installation

1. **Clone the repository:**
   ```sh
   git clone https://github.com/TheSebitzu/linux-memory-allocator-in-c.git
   cd linux-memory-allocator-in-c
   ```

2. **Build the shared library:**
   ```sh
   gcc -o memalloc.so -fPIC -shared memalloc.c -lpthread
   ```
   *(Adjust filenames as needed for your actual implementation)*

3. **(Optional) Compile a test program:**
   ```sh
   gcc -o test_allocator main.c -L. -l:memalloc.so -lpthread
   ```

## Usage

- **Preload the allocator for dynamic linking** (if overriding the standard allocator):
  ```sh
  LD_PRELOAD=$PWD/memalloc.so ./your_program
  ```
- Or, **run a test program** that directly uses your allocator functions:
  ```sh
  ./test_allocator
  ```

- The test program will demonstrate allocation (`malloc`, `calloc`), deallocation (`free`), and resizing (`realloc`).
- You can modify or extend the test program to experiment with various allocation patterns and edge cases.

## How It Works

- The allocator maintains its own linked list or data structure to track allocated and free memory blocks.
- The only system call used to request memory from the OS is `sbrk`.
- On allocation, the allocator finds a suitable free block or expands the heap.
- `calloc` allocates and zero-initializes memory.
- On deallocation, if the freed block is at the end of the heap, the allocator calls `sbrk` with a negative size to shrink the heap and return memory to the OS.
- `realloc` resizes a previously allocated block, copying data if needed.
- All operations are protected with a `pthread` lock for thread safety.

## Limitations

- **Linux only:** Designed for Linux; uses Linux-specific `sbrk` system call.
- **No advanced features:** Does not support memory pools, alignment guarantees, or security features.
- **Educational/demo purposes:** Not intended for production use.

## Requirements

- GCC or any C99-compatible compiler
- **Linux** (uses Linux system calls)
- `pthread` library for thread safety

## License

This project is licensed under the MIT License.

---

*Made by [TheSebitzu](https://github.com/TheSebitzu)*
