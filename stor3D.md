# stor3D
## Block Storage Simulator with HDD / SSD Semantics

---

## I. Foreword

Modern operating systems and databases treat storage devices not as files, but as **block devices**.
The purpose of this project is **not** to reimplement HDDs or SSDs physically, but to implement
the **constraints they impose on software**.

You must demonstrate, using code and execution results, answers to the following:

- Why storage is abstracted as blocks rather than bytes
- Why sequential and random access are fundamentally different on HDDs
- Why SSD overwrite is not a simple in-place write
- Why garbage collection and write amplification are inevitable

---

## II. Goals

- Understand block device abstraction
- Simulate HDD access cost semantics
- Simulate SSD flash constraints (page / block / erase)
- Observe how storage characteristics affect higher-level file operations

---

## III. General Instructions

- Language: **C**
- Norm compliance is mandatory
- Global variables are forbidden
- Memory leaks are forbidden
- Segmentation fault, double free, or undefined behavior results in failure
- Debug prints must be removed before submission
- A Makefile is required and must include:
  - `all`, `clean`, `fclean`, `re`
- Unnecessary relinking is forbidden

---

## IV. Authorized Functions

### Base Functions

- `open`, `close`, `read`, `write`
- `malloc`, `free`
- `gettimeofday`
- `perror`, `strerror`
- `exit`

### Additional Authorized Functions

- `strlen`
- `strnlen`
- `strncmp`
- `strchr`
- `memset`
- `memcpy`
- `memcmp`
- `isdigit`
- `strtol`

> Any function not listed above is strictly forbidden.

---

## V. Program Name & Usage

### Program name
```
stor3D
```

### Usage
```
./stor3D <mode> <disk.img> <script.txt>
```

- `<mode>`: `hdd` or `ssd`
- `<disk.img>`: disk image file
- `<script.txt>`: workload script

---

## VI. Disk Image Specification

- Block size: **4096 bytes**
- Block count: **8192**
- Total size: **32 MB**

Rules:
- If `disk.img` does not exist, it must be created
- If it exists but has an incorrect size, the program must exit with:

```
Error
invalid disk image size
```

---

## VII. Block Device Interface

The following interface must be implemented and used by all upper layers:

```c
int read_block(size_t lba, void *buf);
int write_block(size_t lba, const void *buf);
```

Constraints:
- `lba` must be within `[0, 8192)`
- Partial block access is forbidden

---

## VIII. Script Specification

### Block Commands
- `R <lba>`
- `W <lba> <byte>`

### File Commands
- `CF <name>`
- `WF <name> <offset> <len> <byte>`
- `RF <name> <offset> <len>`
- `CHK <name>`

Rules:
- Tokens separated by spaces
- Empty lines and lines starting with `#` are ignored
- Any invalid line causes immediate error

---

## IX. HDD Mode

### Cost Model

- Seek cost: `abs(prev_lba - curr_lba) * 0.05 ms`
- Rotational latency: `2.0 ms`
- Transfer cost: `0.1 ms`

Costs are accumulated numerically; no real delays are allowed.

### Required Output
```
[HDD] total_reads=...
[HDD] total_writes=...
[HDD] total_time_ms=...
```

---

## X. SSD Mode

### Flash Model

- Page size: 4096 bytes
- Pages per erase block: 256
- Erase block count: 32

### Constraints

- Overwrite is forbidden
- Each write allocates a new page
- Old pages are marked invalid
- Erase only occurs at block granularity

### Garbage Collection

- Triggered when invalid pages >= 50% in a block
- Select block with highest invalid ratio
- Copy valid pages, then erase

### Required Output
```
[SSD] host_writes=...
[SSD] nand_writes=...
[SSD] erases=...
[SSD] gc_moves=...
[SSD] write_amp=...
```

---

## XI. FS_LITE

- Max files: 64
- Metadata stored in a fixed table
- Data stored as extent lists
- Free block management required
- Overwrite must be supported

---

## XII. Error Handling

All errors must be printed as:

```
Error
<explicit message>
```

Recommended messages include:
- `invalid disk image size`
- `invalid mode`
- `cannot open disk image`
- `cannot open script`
- `invalid script line <n>`
- `invalid block address`
- `no space left on device`

---

## XIII. Forbidden Behavior

- Treating disk as a simple array
- Allowing SSD overwrite
- Mixing HDD/SSD logic into upper layers
- Using forbidden functions
- Ignoring parsing errors

---

## XIV. Bonus (Optional)

- TRIM / TRIMF commands
- HDD read-ahead cache
- Script generator

---

## XV. Final Remark

This project evaluates whether you understand
**how storage constraints shape software design**.

Not implementing constraints correctly is considered failure.
