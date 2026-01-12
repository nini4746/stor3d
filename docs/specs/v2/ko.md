# stor3D v2
## 현실적인 블록 스토리지 시뮬레이터

---

## I. 서문

이 프로젝트는 HDD와 SSD 장치의 **물리적 제약과 동작 방식을 정확히 모델링**하는 현실적인 스토리지 시뮬레이터를 구현합니다.

단순화된 교육용 모델과 달리, 이 시뮬레이터는 다음을 요구합니다:

- **정밀한 물리 모델링**: 디스크 기하학 구조 (실린더, 헤드, 섹터)
- **현실적인 성능 특성**: Seek time, 회전 지연, Zone 변화
- **실제 SSD 제약**: Wear leveling, Over-provisioning, 고급 GC
- **관찰 가능한 성능 차이**: 워크로드 패턴에 따른 성능 차이

목표는 실제 하드웨어처럼 동작하는 스토리지 시뮬레이터를 만들어, 스토리지 아키텍처가 시스템 설계를 근본적으로 어떻게 바꾸는지 입증하는 것입니다.

---

## II. 목표

- 현실적인 HDD 물리 기하학 구현 (CHS 주소 지정)
- Zone Bit Recording 및 Read-ahead 캐싱 모델링
- 실제 SSD 플래시 제약 시뮬레이션 (overwrite 불가, wear leveling)
- 프로덕션급 가비지 컬렉션 전략 구현
- 서로 다른 스토리지 타입의 측정 가능한 성능 특성 입증

---

## III. 일반 규칙

- 언어: **C 전용**
- Norm 규칙 준수 필수
- 전역 변수 금지
- 메모리 누수, segfault, double free, undefined behavior 금지
- Makefile 필수: `all`, `clean`, `fclean`, `re`
- 불필요한 relinking 금지

---

## IV. 허용 함수

### 기본 함수
- `open`, `close`, `read`, `write`, `lseek`
- `malloc`, `free`
- `gettimeofday`
- `perror`, `strerror`
- `exit`

### 추가 함수
- `strlen`, `strnlen`, `strncmp`, `strchr`
- `memset`, `memcpy`, `memcmp`
- `isdigit`, `strtol`

> **위 목록에 없는 함수는 엄격히 금지됩니다.**

---

## V. 프로그램 사용법

```bash
./stor3D <mode> <disk.img> <script.txt>
```

- `<mode>`: `hdd` 또는 `ssd`
- `<disk.img>`: 32MB 디스크 이미지 파일
- `<script.txt>`: 워크로드 스크립트

---

## VI. 디스크 이미지 규격

- 블록 크기: **4096 bytes**
- 블록 개수: **8192 blocks**
- 전체 크기: **32 MB** (33,554,432 bytes)

**규칙:**
- 없으면 자동 생성
- 크기 불일치 시 에러: `invalid disk image size`

---

## VII. 블록 디바이스 인터페이스

모든 상위 계층은 이 인터페이스를 사용해야 합니다:

```c
int read_block(t_context *ctx, size_t lba, void *buf);
int write_block(t_context *ctx, size_t lba, const void *buf);
```

- LBA 범위: `[0, 8192)`
- 부분 블록 접근 금지

---

## VIII. 스크립트 명령

### 블록 레벨 명령
```
R <lba>                    # LBA에서 블록 읽기
W <lba> <byte>             # 바이트로 채워진 블록 쓰기
```

### 파일 레벨 명령
```
CF <name>                  # 파일 생성
WF <name> <offset> <len> <byte>  # 파일에 쓰기
RF <name> <offset> <len>   # 파일에서 읽기
CHK <name>                 # 체크섬 계산
TRIMF <name>               # 파일 TRIM (SSD 전용)
```

**규칙:**
- 공백으로 구분된 토큰
- `#` 주석 및 빈 줄 무시
- 잘못된 줄 → 즉시 에러

---

## IX. HDD 모드 - 물리 기하학 모델

### 9.1 디스크 기하학 (CHS 모델)

디스크는 다음과 같이 구성됩니다:

```
실린더(Cylinders): 4
헤드(Heads, 실린더당 트랙 수): 16
트랙당 섹터 수: 가변 (ZBR - 아래 참조)
섹터 크기: 4096 bytes
```

**LBA → CHS 변환:**
```c
// zone의 sectors_per_track을 가정
cylinder = lba / (sectors_per_track * heads)
head = (lba / sectors_per_track) % heads
sector = lba % sectors_per_track
```

### 9.2 Zone Bit Recording (ZBR)

디스크는 섹터 밀도가 다른 **3개 zone**으로 나뉩니다:

| Zone | 실린더 | 트랙당 섹터 | 총 블록 수 |
|------|--------|------------|-----------|
| 0 (바깥) | 0-1 | 256 | 8192 |
| 1 (중간) | 2 | 256 | 4096 |
| 2 (안쪽) | 3 | 256 | 4096 |

**전송 속도** (블록당):
- Zone 0: 0.08 ms
- Zone 1: 0.10 ms
- Zone 2: 0.12 ms

### 9.3 접근 비용 모델

각 I/O 작업마다 누적:

**1. 실린더 Seek**
```
seek_time = abs(curr_cylinder - prev_cylinder) * 0.5 ms
```

**2. 헤드 전환**
```
head_switch_time = abs(curr_head - prev_head) * 0.1 ms
```

**3. 회전 지연**
```
// 7200 RPM 가정 (회전당 8.33ms)
rotation_time = 4.17 ms (평균: 반 회전)
```

**4. 전송 시간**
```
transfer_time = zone_transfer_time[zone]
```

### 9.4 Read-Ahead 캐시

**8블록** read-ahead 캐시 구현:

- **순차 감지**: 3개 이상 연속 LBA
- **캐시 동작**:
  - 순차 읽기 시, 다음 8블록 미리 읽기
  - 캐시 히트 → seek + rotation 비용 생략 (전송만)
  - 캐시 미스 → 전체 접근 비용
- **교체 정책**: LRU (Least Recently Used)

### 9.5 HDD 출력 형식

```
[HDD] total_reads=<개수>
[HDD] total_writes=<개수>
[HDD] cylinder_seeks=<개수>
[HDD] head_switches=<개수>
[HDD] cache_hits=<개수>
[HDD] cache_misses=<개수>
[HDD] total_time_ms=<시간>
```

---

## X. SSD 모드 - 플래시 메모리 모델

### 10.1 플래시 기하학

```
페이지 크기: 4096 bytes
Erase block당 페이지 수: 256
Erase block 개수: 32
물리적 용량: 32 MB
```

### 10.2 Over-Provisioning

용량의 **90%만** 사용자가 접근 가능:

- **사용자 용량**: 7372 blocks (28.8 MB)
- **OP 영역**: 820 blocks (3.2 MB)
- **목적**: GC 성능 및 wear leveling 향상

### 10.3 핵심 제약

1. **제자리 Overwrite 금지**
   - 각 write는 새 물리 페이지 할당
   - 이전 페이지는 invalid 마킹

2. **Erase Block 단위 소거**
   - 256페이지 블록 전체만 소거 가능
   - 소거 전 valid 페이지를 복사해야 함

3. **Wear Leveling**
   - 블록별 erase count 추적
   - 새 할당 시 낮은 erase count 블록 선호

### 10.4 Flash Translation Layer (FTL)

**매핑 테이블** 유지:

```c
typedef struct s_ftl_entry {
    size_t ppa;          // Physical Page Address
    int    valid;        // 1: valid, 0: invalid
} t_ftl_entry;

t_ftl_entry ftl_map[8192];  // LBA → PPA 매핑
```

### 10.5 페이지/블록 관리

```c
typedef struct s_flash_block {
    int pages[256];           // 페이지 상태: 0=free, 1=valid, -1=invalid
    int valid_count;          // valid 페이지 수
    int invalid_count;        // invalid 페이지 수
    int erase_count;          // 이 블록이 소거된 횟수
} t_flash_block;

t_flash_block blocks[32];
```

### 10.6 가비지 컬렉션

**GC 트리거**:
- 여유 페이지 < OP 영역의 10% (82 페이지)
- 또는 여유 페이지 부족으로 write 실패 시

**GC 알고리즘** (선택):

**Option A: Greedy**
```
invalid 페이지 수가 가장 많은 블록 선택
```

**Option B: Cost-Benefit** (권장)
```
score = (invalid_count / total_pages) / (2 * erase_count)
가장 높은 점수의 블록 선택
```

**GC 프로세스**:
1. victim 블록 선택
2. 모든 valid 페이지를 새 위치로 복사 (gc_moves++)
3. FTL 매핑 업데이트
4. victim 블록 소거 (erases++, erase_count++)

### 10.7 Wear Leveling

**free 블록** 할당 시:
- 가장 낮은 erase_count를 가진 블록 선호
- 모든 블록에 쓰기를 고르게 분산

### 10.8 TRIM 지원

**TRIMF 명령**:
- 파일에 속한 모든 페이지를 invalid로 마킹
- 즉시 GC를 트리거하지 않음
- 향후 write amplification 감소

### 10.9 SSD 성능 비용

```
Read:  0.05 ms per page
Write: 0.1 ms per page
Erase: 1.5 ms per block
```

### 10.10 SSD 출력 형식

```
[SSD] host_writes=<개수>
[SSD] nand_writes=<개수>
[SSD] erases=<개수>
[SSD] gc_count=<개수>
[SSD] gc_moves=<개수>
[SSD] write_amp=<비율>
[SSD] max_erase_count=<개수>
[SSD] min_erase_count=<개수>
[SSD] total_time_ms=<시간>
```

**Write Amplification**:
```c
write_amp = (double)nand_writes / host_writes
```

---

## XI. FS_LITE - 간단한 파일시스템

- **최대 파일 수**: 64
- **메타데이터**: 블록 0에 고정 크기 테이블
- **데이터 저장**: Extent 기반 할당
- **여유 블록 관리**: Bitmap 또는 연결 리스트
- **Overwrite 지원**: 필수 (SSD에서는 새 할당 트리거)

### 메타데이터 구조 예시

```c
typedef struct s_file_meta {
    char    name[32];
    size_t  size;           // 파일 크기 (bytes)
    size_t  extent_start;   // 시작 LBA
    size_t  extent_count;   // 블록 개수
    int     valid;          // 1: 존재, 0: 삭제됨
} t_file_meta;

t_file_meta file_table[64];  // 블록 0에 저장
```

---

## XII. 에러 처리

`perror()`를 사용하여 명확한 메시지 출력:

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

## XIII. 구현 요구사항

### HDD 필수 구현:
✅ CHS 기하학 및 주소 지정
✅ Zone Bit Recording (3 zones)
✅ 정확한 seek/head/rotation 비용
✅ 8블록 read-ahead 캐시 (LRU)
✅ 순차 접근 감지
✅ 상세한 통계 출력

### SSD 필수 구현:
✅ LBA→PPA 매핑을 가진 FTL
✅ 제자리 overwrite 금지
✅ 90% 사용자 용량 (10% OP)
✅ Wear leveling (erase count 추적)
✅ 가비지 컬렉션 (Greedy 또는 Cost-Benefit)
✅ TRIMF 명령 지원
✅ Write amplification 계산
✅ 상세한 통계 출력

---

## XIV. 금지 사항

❌ 디스크를 단순 배열처럼 취급
❌ SSD 제자리 overwrite 허용
❌ HDD/SSD 로직을 상위 계층에 혼합
❌ 금지된 함수 사용
❌ 물리적 제약 무시
❌ 단순화된 비용 모델

---

## XV. 예상 성능 차이

구현은 다음을 명확히 입증해야 합니다:

### HDD:
- 순차 접근: 빠름 (최소 seek)
- 랜덤 접근: 느림 (많은 seek)
- 캐시 히트: 극적인 속도 향상

### SSD:
- 랜덤 접근: 빠름 (seek 페널티 없음)
- Write amplification: 워크로드에 따라 1.2-3.0×
- GC 오버헤드: 쓰기에 측정 가능한 영향
- TRIM 효과: write amplification 감소

---

## XVI. 제출물

1. **소스 코드** (norm 완전 준수)
2. **Makefile** (all, clean, fclean, re)
3. **테스트 스크립트** 다음을 입증:
   - 순차 vs 랜덤 HDD 성능
   - 캐시 hit/miss 동작
   - SSD write amplification
   - GC 트리거 및 실행
   - TRIM 효과

---

## XVII. 평가 기준

- **정확성**: 모든 제약이 정확히 구현됨
- **현실성**: 성능이 예상 물리적 동작과 일치
- **코드 품질**: 깔끔한 아키텍처, 누수 없음, norm 준수
- **관찰 가능성**: 차이를 입증하는 명확한 통계
- **문서화**: 물리 모델을 설명하는 코드 주석

---

## XVIII. 권장 프로젝트 구조

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

## XIX. 최종 참고사항

이것은 교육용 장난감이 아닙니다—**현실적인 스토리지 시뮬레이터**입니다.

코드는 다음에 대한 깊은 이해를 입증해야 합니다:
- HDD가 순차 접근을 선호하는 이유
- SSD가 제자리 overwrite를 할 수 없는 이유
- 가비지 컬렉션이 성능에 미치는 영향
- Wear leveling이 수명에 중요한 이유
- Over-provisioning이 SSD 성능을 향상시키는 방식

**정밀하게 구현하고. 철저히 테스트하고. 꼼꼼히 문서화하십시오.**
