# CLAUDE.md - stor3D Project Guide

## Project Overview
**stor3D**: Block Storage Simulator with HDD/SSD Semantics

This project simulates HDD and SSD storage devices at the block level to demonstrate how storage constraints shape software design.

## Key Constraints
- Language: **C only**
- Norm compliance mandatory
- No global variables
- No memory leaks
- No segfaults, double free, or undefined behavior

## Program Specification

### Usage
```bash
./stor3D <mode> <disk.img> <script.txt>
```
- `<mode>`: `hdd` or `ssd`
- `<disk.img>`: disk image file (32 MB)
- `<script.txt>`: workload script

### Disk Parameters
- Block size: 4096 bytes
- Block count: 8192 blocks
- Total size: 32 MB

## Architecture Layers

### 1. Block Device Interface (Core Abstraction)
```c
int read_block(size_t lba, void *buf);
int write_block(size_t lba, const void *buf);
```
- LBA range: [0, 8192)
- No partial block access allowed

### 2. HDD Mode
**Cost Model** (accumulated, no real delays):
- Seek: `abs(prev_lba - curr_lba) * 0.05 ms`
- Rotational latency: `2.0 ms`
- Transfer: `0.1 ms`

**Output**:
```
[HDD] total_reads=...
[HDD] total_writes=...
[HDD] total_time_ms=...
```

### 3. SSD Mode
**Flash Model**:
- Page size: 4096 bytes
- Pages per erase block: 256
- Erase block count: 32

**Constraints**:
- No overwrite allowed
- Each write allocates new page
- Old pages marked invalid
- Erase at block granularity only

**Garbage Collection**:
- Trigger: invalid pages >= 50% in a block
- Strategy: select block with highest invalid ratio
- Action: copy valid pages, then erase

**Output**:
```
[SSD] host_writes=...
[SSD] nand_writes=...
[SSD] erases=...
[SSD] gc_moves=...
[SSD] write_amp=...
```

### 4. FS_LITE (Simple Filesystem)
- Max files: 64
- Fixed metadata table
- Extent-based data storage
- Free block management required
- Must support overwrite

## Script Commands

### Block Commands
- `R <lba>` - Read block
- `W <lba> <byte>` - Write block filled with byte

### File Commands
- `CF <name>` - Create file
- `WF <name> <offset> <len> <byte>` - Write to file
- `RF <name> <offset> <len>` - Read from file
- `CHK <name>` - Checksum file

**Rules**:
- Space-separated tokens
- Empty lines and `#` comments ignored
- Invalid line = immediate error

## Error Handling
All errors use `perror()` with descriptive messages:

```c
perror("usage : ./stor3D <mode> <disk.img> <script.txt>");
perror("invalid disk image size");
perror("mode is only hdd, ssd");
perror("cannot open disk image");
perror("cannot open script");
perror("invalid script line");
perror("invalid block address");
perror("no space left on device");
```

**Format**: Always use `perror()` for consistent error reporting

## Authorized Functions

### Base
- `open`, `close`, `read`, `write`
- `malloc`, `free`
- `gettimeofday`
- `perror`, `strerror`
- `exit`

### Additional
- `strlen`, `strnlen`, `strncmp`, `strchr`
- `memset`, `memcpy`, `memcmp`
- `isdigit`, `strtol`

> **Any function not listed is FORBIDDEN**

## Required Definitions (stor3d.h)

```c
// Disk specifications
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 8192
#define DISK_SIZE (BLOCK_SIZE * BLOCK_COUNT)  // 33554432 bytes = 32MB

// FS_LITE
#define MAX_FILES 64

// SSD Flash Model
#define PAGE_SIZE 4096
#define PAGES_PER_BLOCK 256
#define ERASE_BLOCK_COUNT 32
#define SSD_GC_THRESHOLD 0.5

// HDD Cost Model (in milliseconds)
#define HDD_SEEK_COST 0.05
#define HDD_ROTATIONAL_LATENCY 2.0
#define HDD_TRANSFER_COST 0.1

// Mode enumeration
typedef enum e_mode
{
    MODE_HDD,
    MODE_SSD
}   t_mode;

// Runtime context structure
typedef struct s_context
{
    int         disk_fd;        // Disk image file descriptor
    int         script_fd;      // Script file descriptor
    t_mode      mode;           // HDD or SSD mode
    size_t      total_reads;    // Total read operations
    size_t      total_writes;   // Total write operations
    double      total_time_ms;  // HDD: accumulated time
    long        prev_lba;       // HDD: previous LBA for seek calculation
    size_t      host_writes;    // SSD: writes from host
    size_t      nand_writes;    // SSD: actual NAND writes
    size_t      erases;         // SSD: erase operations
    size_t      gc_moves;       // SSD: pages moved during GC
}   t_context;
```

## Recommended Project Structure

```
stor3d/
├── Makefile
├── test_stor3d.sh         # Comprehensive tester script
├── include/
│   ├── stor3d.h           # Common definitions
│   ├── block_device.h     # Block device interface
│   ├── hdd.h              # HDD simulation
│   ├── ssd.h              # SSD simulation
│   ├── fs_lite.h          # Filesystem layer
│   ├── script_parser.h    # Script parsing
│   └── utils.h            # Utilities
└── src/
    ├── main.c             # Entry point
    ├── validation/
    │   └── validation.c   # Input validation
    ├── block_device.c     # Block device implementation
    ├── hdd.c              # HDD logic
    ├── ssd.c              # SSD logic
    ├── fs_lite.c          # Filesystem logic
    ├── script_parser.c    # Script execution
    └── utils.c            # Helper functions
```

## Critical Implementation Notes

### DO NOT:
- Treat disk as simple array
- Allow SSD overwrite
- Mix HDD/SSD logic into upper layers
- Use forbidden functions
- Ignore parsing errors

### DO:
- Maintain clean layer separation
- Implement proper error handling
- Follow block device abstraction strictly
- Demonstrate why constraints exist through code

## Implementation Priority

1. **Block Device Layer** - Core abstraction
2. **Disk Image Management** - Create/validate 32MB file
3. **HDD Mode** - Cost tracking
4. **Script Parser** - Block commands (R, W)
5. **SSD Mode** - Flash translation layer + GC
6. **FS_LITE** - Filesystem layer
7. **File Commands** - CF, WF, RF, CHK

## Key Implementation Examples

### Disk Image Validation (validation.c)

**Important**: This is a **disk image** file, NOT a Docker image. It's a plain 32MB binary file that simulates physical storage.

#### validate_image() - Main validation entry
```c
int valid_image(const char *image_path)
{
    int fd;
    long size;

    fd = open(image_path, O_RDWR);

    if (fd < 0)
    {
        // File doesn't exist - create it
        if (create_image(image_path))
            return (1);
    }
    else
    {
        // File exists - validate size
        size = lseek(fd, 0, SEEK_END);
        if (size != DISK_SIZE)
        {
            perror("invalid disk image size");
            close(fd);
            return (1);
        }
        close(fd);
    }
    return (0);
}
```

#### create_image() - Create 32MB disk image
```c
int create_image(const char *image_path)
{
    int fd;
    char buf[BLOCK_SIZE];
    size_t i;

    fd = open(image_path, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("cannot open disk image");
        return (1);
    }

    memset(buf, 0, BLOCK_SIZE);
    i = 0;
    while (i < BLOCK_COUNT)
    {
        if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
        {
            perror("cannot create disk image");
            close(fd);
            return (1);
        }
        i++;
    }
    close(fd);
    return (0);
}
```

#### valid_script() - Script file validation
```c
int valid_script(const char *script_path)
{
    int fd;

    fd = open(script_path, O_RDONLY);
    if (fd < 0)
    {
        perror("cannot open script");
        return (1);
    }
    close(fd);
    return (0);
}
```

**Note**: Actual script parsing and command validation happens in `script_parser.c` during execution.

### Validation Flow (is_valid)

```c
int is_valid(int argc, char **argv)
{
    // 1. Argument count
    if (argc != 4)
    {
        perror("usage : ./stor3D <mode> <disk.img> <script.txt>");
        return (1);
    }

    // 2. Mode validation (only "hdd" or "ssd")
    if (strcmp(argv[1], "hdd") != 0 && strcmp(argv[1], "ssd") != 0)
    {
        perror("mode is only hdd, ssd");
        return (1);
    }

    // 3. Disk image validation
    if (valid_image(argv[2]))
        return (1);

    // 4. Script file validation
    if (valid_script(argv[3]))
        return (1);

    return (0);
}
```

### Context Initialization (init.c)

**Important**: Functions must not exceed 25 lines (42 norm). Split initialization logic into helper functions.

#### open_context_files() - Helper function

**Note**: Do NOT use `static` during development. Add `static` only in final cleanup phase.

```c
// TODO: static으로 변경 예정
int	open_context_files(t_context *ctx, char **argv)
{
	ctx->disk_fd = open(argv[2], O_RDWR);
	if (ctx->disk_fd < 0)
	{
		perror("cannot open disk image");
		free(ctx);
		return (1);
	}
	ctx->script_fd = open(argv[3], O_RDONLY);
	if (ctx->script_fd < 0)
	{
		perror("cannot open script");
		close(ctx->disk_fd);
		free(ctx);
		return (1);
	}
	return (0);
}
```

#### init_context() - Main initialization
```c
int	init_context(t_context **ctx, char **argv)
{
	*ctx = (t_context *)malloc(sizeof(t_context));
	if (!*ctx)
	{
		perror("malloc failed");
		return (1);
	}
	memset(*ctx, 0, sizeof(t_context));
	if (strcmp(argv[1], "hdd") == 0)
		(*ctx)->mode = MODE_HDD;
	else
		(*ctx)->mode = MODE_SSD;
	(*ctx)->prev_lba = -1;
	if (open_context_files(*ctx, argv))
		return (1);
	return (0);
}
```

#### cleanup_context() - Resource cleanup
```c
void	cleanup_context(t_context *ctx)
{
	if (!ctx)
		return ;
	if (ctx->disk_fd >= 0)
		close(ctx->disk_fd);
	if (ctx->script_fd >= 0)
		close(ctx->script_fd);
	free(ctx);
}
```

**Key Points**:
- Use `enum` for mode instead of storing string pointer
- Initialize `prev_lba` to `-1` for HDD tracking
- Split into helper functions to respect 25-line limit
- Proper resource cleanup on error

## Testing

### Comprehensive Test Suite (test_stor3d.sh)

A comprehensive test script is provided with **100+ test cases** covering:

**Basic Validation (8 tests)**
- Argument count validation
- Mode validation (hdd/ssd only)
- Disk image creation and size validation
- Script file validation

**Statistics Tests (10 tests)**
- HDD/SSD output format verification
- Read/write count accuracy
- HDD timing calculations (seek, rotational latency, transfer)
- SSD write amplification and GC metrics

**Filesystem Tests (8 tests)**
- CF, WF, RF, CHK commands
- Error handling (non-existent files, duplicates, max files)

**Edge Cases (6 tests)**
- Byte value boundaries (0x00, 0xFF)
- Long scripts (500+ commands)
- Comment-only scripts
- Performance patterns (sequential vs random)

**EVIL TESTS (68 tests)** 🔥
- Disk full scenarios
- SSD GC stress (1000 overwrites on same LBA)
- Whitespace hell (tabs, spaces, mixed)
- Number format variations (hex case, overflow)
- Filesystem boundaries (negative offsets, huge values)
- SSD page/block boundaries
- HDD worst-case seek patterns
- Command variations (case sensitivity)
- Data integrity verification
- Resource exhaustion (64 files, full disk)

**Usage:**
```bash
./test_stor3d.sh
```

**Output:** Color-coded PASS/FAIL results with final statistics.

## Important Implementation Notes

### Disk Image vs Docker Image
- **Disk image** = Plain binary file (like a virtual hard drive)
- **NOT** a Docker container image
- Created with `write()` in C or `dd` command
- Simply a 32MB file filled with zeros initially

### Error Handling Consistency
- **Always use `perror()`** for error messages
- No custom error printing functions
- Format: `perror("descriptive message");`
- Exit with non-zero code on error

### Memory Management
- No global variables allowed
- All allocations must be freed
- Check for leaks with valgrind
- No double-free or use-after-free

### lseek() Usage
- Not in authorized list but commonly allowed with `open/read/write`
- Used for: getting file size, seeking to position
- Example: `lseek(fd, 0, SEEK_END)` returns file size

### Function Restrictions
- **Only use authorized functions**
- No `printf`, `fprintf` - use `perror` for errors
- No `strdup` - manually allocate with `malloc`
- No `atoi` - use `strtol` instead

## Bonus Features (Optional)
- TRIM / TRIMF commands
- HDD read-ahead cache
- Script generator

## Project Goal
Demonstrate understanding of **how storage constraints shape software design** through working code and execution results.

---

## Quick Start Checklist

- [ ] Compile with Makefile (all, clean, fclean, re)
- [ ] Program accepts: `./stor3D <mode> <disk.img> <script.txt>`
- [ ] Mode is exactly "hdd" or "ssd" (lowercase)
- [ ] Creates disk.img if missing (32MB = 33554432 bytes)
- [ ] Rejects disk.img if wrong size
- [ ] Parses script: R/W for blocks, CF/WF/RF/CHK for files
- [ ] Implements block device interface (read_block/write_block)
- [ ] HDD outputs: total_reads, total_writes, total_time_ms
- [ ] SSD outputs: host_writes, nand_writes, erases, gc_moves, write_amp
- [ ] All errors use perror()
- [ ] No memory leaks, no crashes
- [ ] Run test_stor3d.sh to verify
