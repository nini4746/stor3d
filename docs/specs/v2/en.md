# stor3D v2
## Realistic Block Storage Simulator

---

## I. Foreword

This project implements a **realistic storage simulator** that accurately models the physical constraints and behaviors of HDD and SSD devices.

Unlike simplified educational models, this simulator requires:

- **Precise physical modeling** of disk geometry (cylinders, heads, sectors)
- **Realistic performance characteristics** (seek times, rotational delays, zone variations)
- **Authentic SSD constraints** (wear leveling, over-provisioning, advanced GC)
- **Observable performance differences** between workload patterns

The goal is to build a storage simulator that behaves like real hardware, demonstrating why storage architecture fundamentally shapes system design.

---

## II. Goals

- Implement realistic HDD physical geometry (CHS addressing)
- Model Zone Bit Recording and read-ahead caching
- Simulate authentic SSD flash constraints (no overwrite, wear leveling)
- Implement production-grade garbage collection strategies
- Demonstrate measurable performance characteristics of different storage types

---

## III. General Instructions

- Language: **C only**
- Norm compliance is mandatory
- No global variables
- No memory leaks, segfaults, double frees, or undefined behavior
- A Makefile is required: `all`, `clean`, `fclean`, `re`
- No unnecessary relinking

---

## IV. Authorized Functions

### Base Functions
- `open`, `close`, `read`, `write`, `lseek`
- `malloc`, `free`
- `gettimeofday`
- `perror`, `strerror`
- `exit`

### Additional Functions
- `strlen`, `strnlen`, `strncmp`, `strchr`
- `memset`, `memcpy`, `memcmp`
- `isdigit`, `strtol`

> **Any function not listed is strictly forbidden.**

---

## V. Program Usage

```bash
./stor3D <mode> <disk.img> <script.txt>
```

- `<mode>`: `hdd` or `ssd`
- `<disk.img>`: 32MB disk image file
- `<script.txt>`: workload script

---

## VI. Disk Image Specification

- Block size: **4096 bytes**
- Block count: **8192 blocks**
- Total size: **32 MB** (33,554,432 bytes)

**Rules:**
- Auto-create if missing
- Reject if wrong size with error: `invalid disk image size`

---

## VII. Block Device Interface

All upper layers must use this interface:

```c
int read_block(t_context *ctx, size_t lba, void *buf);
int write_block(t_context *ctx, size_t lba, const void *buf);
```

- LBA range: `[0, 8192)`
- No partial block access

---

## VIII. Script Commands

### Block-Level Commands
```
R <lba>                    # Read block at LBA
W <lba> <byte>             # Write block filled with byte
```

### File-Level Commands
```
CF <name>                  # Create file
WF <name> <offset> <len> <byte>  # Write to file
RF <name> <offset> <len>   # Read from file
CHK <name>                 # Compute checksum
TRIMF <name>               # TRIM file (SSD only)
```

**Rules:**
- Space-separated tokens
- `#` comments and empty lines ignored
- Invalid line → immediate error

---

## IX. HDD Mode - Physical Geometry Model

### 9.1 Disk Geometry (CHS Model)

The disk is organized as:

```
Cylinders: 4
Heads (tracks per cylinder): 16
Sectors per track: Variable (ZBR - see below)
Sector size: 4096 bytes
```

**LBA to CHS Conversion:**
```c
// Assuming sectors_per_track for the zone
cylinder = lba / (sectors_per_track * heads)
head = (lba / sectors_per_track) % heads
sector = lba % sectors_per_track
```

### 9.2 Zone Bit Recording (ZBR)

The disk is divided into **3 zones** with different sector densities:

| Zone | Cylinders | Sectors/Track | Total Blocks |
|------|-----------|---------------|--------------|
| 0 (outer) | 0-1 | 256 | 8192 |
| 1 (middle) | 2 | 256 | 4096 |
| 2 (inner) | 3 | 256 | 4096 |

**Transfer speeds** (per block):
- Zone 0: 0.08 ms
- Zone 1: 0.10 ms
- Zone 2: 0.12 ms

### 9.3 Access Cost Model

For each I/O operation, accumulate:

**1. Cylinder Seek**
```
seek_time = abs(curr_cylinder - prev_cylinder) * 0.5 ms
```

**2. Head Switch**
```
head_switch_time = abs(curr_head - prev_head) * 0.1 ms
```

**3. Rotational Delay**
```
// Assume 7200 RPM (8.33ms per rotation)
rotation_time = 4.17 ms (average: half rotation)
```

**4. Transfer Time**
```
transfer_time = zone_transfer_time[zone]
```

### 9.4 Read-Ahead Cache

Implement an **8-block** read-ahead cache:

- **Sequential detection**: 3+ consecutive LBAs
- **Cache behavior**:
  - On sequential read, prefetch next 8 blocks
  - Cache hit → skip seek + rotation costs (only transfer)
  - Cache miss → full access cost
- **Replacement**: LRU (Least Recently Used)

### 9.5 HDD Output Format

```
[HDD] total_reads=<count>
[HDD] total_writes=<count>
[HDD] cylinder_seeks=<count>
[HDD] head_switches=<count>
[HDD] cache_hits=<count>
[HDD] cache_misses=<count>
[HDD] total_time_ms=<time>
```

---

## X. SSD Mode - Flash Memory Model

### 10.1 Flash Geometry

```
Page size: 4096 bytes
Pages per erase block: 256
Erase block count: 32
Physical capacity: 32 MB
```

### 10.2 Over-Provisioning

Only **90% of capacity** is user-accessible:

- **User capacity**: 7372 blocks (28.8 MB)
- **OP area**: 820 blocks (3.2 MB)
- **Purpose**: Improve GC performance and wear leveling

### 10.3 Core Constraints

1. **No In-Place Overwrite**
   - Each write allocates a new physical page
   - Old page marked invalid

2. **Erase Block Granularity**
   - Can only erase entire 256-page blocks
   - Must copy out valid pages before erasing

3. **Wear Leveling**
   - Track erase count per block
   - Prefer blocks with lower erase counts for new allocations

### 10.4 Flash Translation Layer (FTL)

Maintain a **mapping table**:

```c
typedef struct s_ftl_entry {
    size_t ppa;          // Physical Page Address
    int    valid;        // 1: valid, 0: invalid
} t_ftl_entry;

t_ftl_entry ftl_map[8192];  // LBA → PPA mapping
```

### 10.5 Page/Block Management

```c
typedef struct s_flash_block {
    int pages[256];           // Page states: 0=free, 1=valid, -1=invalid
    int valid_count;          // Number of valid pages
    int invalid_count;        // Number of invalid pages
    int erase_count;          // Times this block has been erased
} t_flash_block;

t_flash_block blocks[32];
```

### 10.6 Garbage Collection

**GC Trigger**:
- When free pages < 10% of OP area (82 pages)
- OR when write fails due to no free pages

**GC Algorithm** (choose one):

**Option A: Greedy**
```
Select block with highest invalid page count
```

**Option B: Cost-Benefit** (recommended)
```
score = (invalid_count / total_pages) / (2 * erase_count)
Select block with highest score
```

**GC Process**:
1. Select victim block
2. Copy all valid pages to new locations (gc_moves++)
3. Update FTL mapping
4. Erase victim block (erases++, erase_count++)

### 10.7 Wear Leveling

When selecting a **free block** for allocation:
- Prefer blocks with lowest erase_count
- Helps distribute writes evenly across all blocks

### 10.8 TRIM Support

**TRIMF command**:
- Mark all pages belonging to the file as invalid
- Does NOT trigger immediate GC
- Reduces future write amplification

### 10.9 SSD Performance Costs

```
Read:  0.05 ms per page
Write: 0.1 ms per page
Erase: 1.5 ms per block
```

### 10.10 SSD Output Format

```
[SSD] host_writes=<count>
[SSD] nand_writes=<count>
[SSD] erases=<count>
[SSD] gc_count=<count>
[SSD] gc_moves=<count>
[SSD] write_amp=<ratio>
[SSD] max_erase_count=<count>
[SSD] min_erase_count=<count>
[SSD] total_time_ms=<time>
```

**Write Amplification**:
```c
write_amp = (double)nand_writes / host_writes
```

---

## XI. FS_LITE - Simple Filesystem

- **Max files**: 64
- **Metadata**: Fixed-size table at block 0
- **Data storage**: Extent-based allocation
- **Free block management**: Bitmap or linked list
- **Overwrite support**: Required (triggers new allocations in SSD)

### Metadata Structure Example

```c
typedef struct s_file_meta {
    char    name[32];
    size_t  size;           // File size in bytes
    size_t  extent_start;   // Starting LBA
    size_t  extent_count;   // Number of blocks
    int     valid;          // 1: exists, 0: deleted
} t_file_meta;

t_file_meta file_table[64];  // Stored in block 0
```

---

## XII. Error Handling

Use `perror()` with descriptive messages:

```c
perror("usage : ./stor3D <mode> <disk.img> <script.txt>");
perror("invalid disk image size");
perror("mode is only hdd, ssd");
perror("cannot open disk image");
perror("cannot open script");
perror("invalid script line");
perror("invalid block address");
perror("no space left on device");
perror("file not found");
perror("file already exists");
perror("max files reached");
```

---

## XIII. Implementation Requirements

### HDD Must Implement:
✅ CHS geometry and addressing
✅ Zone Bit Recording (3 zones)
✅ Accurate seek/head/rotation costs
✅ 8-block read-ahead cache with LRU
✅ Sequential access detection
✅ Detailed statistics output

### SSD Must Implement:
✅ FTL with LBA→PPA mapping
✅ No in-place overwrite
✅ 90% user capacity (10% OP)
✅ Wear leveling (erase count tracking)
✅ Garbage collection (Greedy or Cost-Benefit)
✅ TRIMF command support
✅ Write amplification calculation
✅ Detailed statistics output

---

## XIV. Forbidden Behaviors

❌ Treating disk as simple array
❌ Allowing SSD in-place overwrite
❌ Mixing HDD/SSD logic into upper layers
❌ Using forbidden functions
❌ Ignoring physical constraints
❌ Simplified cost models

---

## XV. Expected Performance Differences

Your implementation should clearly demonstrate:

### HDD:
- Sequential access: Fast (minimal seeks)
- Random access: Slow (many seeks)
- Cache hits: Dramatic speedup

### SSD:
- Random access: Fast (no seek penalty)
- Write amplification: 1.2-3.0× depending on workload
- GC overhead: Measurable impact on writes
- TRIM benefit: Reduced write amplification

---

## XVI. Deliverables

1. **Source code** (fully compliant with norm)
2. **Makefile** (all, clean, fclean, re)
3. **Test scripts** demonstrating:
   - Sequential vs random HDD performance
   - Cache hit/miss behavior
   - SSD write amplification
   - GC trigger and execution
   - TRIM effectiveness

---

## XVII. Evaluation Criteria

- **Correctness**: All constraints implemented accurately
- **Realism**: Performance matches expected physical behavior
- **Code quality**: Clean architecture, no leaks, norm compliance
- **Observability**: Clear statistics demonstrating differences
- **Documentation**: Code comments explaining physical models

---

## XVIII. Recommended Project Structure

```
stor3d/
├── Makefile
├── include/
│   ├── stor3d.h
│   ├── hdd.h
│   ├── ssd.h
│   ├── fs_lite.h
│   └── cache.h
└── src/
    ├── main.c
    ├── validation/
    │   └── validation.c
    ├── init/
    │   └── init.c
    ├── block/
    │   └── block_device.c
    ├── hdd/
    │   ├── hdd_chs.c
    │   ├── hdd_cost.c
    │   └── hdd_cache.c
    ├── ssd/
    │   ├── ssd_ftl.c
    │   ├── ssd_gc.c
    │   ├── ssd_wear.c
    │   └── ssd_trim.c
    ├── fs_lite/
    │   └── fs_lite.c
    └── parsing/
        └── parser.c
```

---

## XIX. Final Note

This is not an educational toy—it's a **realistic storage simulator**.

Your code must demonstrate deep understanding of:
- Why HDDs favor sequential access
- Why SSDs cannot overwrite in place
- How garbage collection affects performance
- Why wear leveling matters for longevity
- How over-provisioning improves SSD performance

**Implement with precision. Test rigorously. Document thoroughly.**
