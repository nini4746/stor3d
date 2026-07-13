# stor3D v2
## 현실적인 블록 스토리지 시뮬레이터

HDD와 SSD의 물리적 제약을 정확히 모델링하는 스토리지 시뮬레이터 프로젝트입니다.

**✅ 프로젝트 완성 (2026-01-14)**
- 모든 필수 기능 구현 완료
- 42 Norm 완전 준수 (25개 파일 모두 통과)
- 메모리 누수 없음, 안정적 동작 확인

---

## 📚 문서

### 프로젝트 명세서
- **[v2 명세 (한글)](./docs/specs/v2/ko.md)** ← 현재 버전
- **[v2 명세 (영문)](./docs/specs/v2/en.md)**
- [v1 명세 (한글)](./docs/specs/v1/ko.md) - 원본 (참고용)
- [v1 명세 (영문)](./docs/specs/v1/en.md) - 원본 (참고용)

### 구현 가이드
- **[구현 가이드](./docs/guides/implementation.md)** - 코드 예제 및 상세 설명

---

## 🚀 빠른 시작

### 컴파일
```bash
make
```

### 실행
```bash
./stor3D <mode> <disk.img> <script.txt>
```

- `<mode>`: `hdd` 또는 `ssd`
- `<disk.img>`: 32MB 디스크 이미지 파일
- `<script.txt>`: 워크로드 스크립트

### 테스트
```bash
./test_stor3d.sh
```

---

## 📁 프로젝트 구조

```
stor3d/
├── README.md                  # 이 파일
├── Makefile                   # 빌드 설정
├── test_basic.txt             # 기본 테스트 스크립트
├── test_fs.txt                # 파일시스템 테스트 스크립트
├── docs/                      # 📚 문서
│   ├── specs/                 # 프로젝트 명세서
│   │   ├── v1/                # 원본 버전
│   │   │   ├── en.md
│   │   │   └── ko.md
│   │   └── v2/                # 현재 버전 ⭐
│   │       ├── en.md
│   │       └── ko.md
│   └── guides/                # 구현 가이드
│       └── implementation.md  # 상세 구현 가이드 및 진행상황
├── include/                   # 헤더 파일 (5개)
│   ├── stor3d.h               # 공통 정의 및 context
│   ├── block_device.h         # Block device interface
│   ├── hdd.h                  # HDD 구조체 및 함수
│   ├── ssd.h                  # SSD 구조체 및 함수
│   ├── fs_lite.h              # 파일시스템 구조체 및 함수
│   └── parser.h               # Parser 함수
└── src/                       # 소스 코드 (21개 파일)
    ├── main.c                 # 진입점
    ├── init/                  # 초기화 (2개)
    │   ├── validation.c       # 입력 검증 (5 funcs)
    │   └── context.c          # Context init/cleanup (3 funcs)
    ├── device/                # 블록 디바이스 (1개)
    │   └── block_device.c     # 블록 I/O 추상화 (3 funcs)
    ├── parser/                # 스크립트 파싱 (4개)
    │   ├── parser.c           # 파서 메인 (5 funcs)
    │   ├── commands.c         # R/W 명령 (2 funcs)
    │   ├── file_commands.c    # CF/WF/RF/CHK 명령 (4 funcs)
    │   └── utils.c            # 파싱 유틸 (3 funcs)
    ├── hdd/                   # HDD 구현 (4개)
    │   ├── hdd_init.c         # 초기화 및 stats (5 funcs)
    │   ├── hdd_helpers.c      # CHS, 비용 계산 (5 funcs)
    │   ├── hdd_cache.c        # LRU 캐시 (4 funcs)
    │   └── hdd_io.c           # 물리 I/O (2 funcs)
    ├── ssd/                   # SSD 구현 (5개)
    │   ├── ssd_init.c         # 초기화 및 stats (5 funcs)
    │   ├── ssd_ftl.c          # FTL 매핑 (4 funcs)
    │   ├── ssd_gc.c           # GC 메인 (4 funcs)
    │   ├── ssd_gc_helpers.c   # GC 헬퍼 (3 funcs)
    │   └── ssd_io.c           # Read/Write (4 funcs)
    └── filesystem/            # FS_LITE (4개)
        ├── fs_init.c          # 초기화 및 lookup (5 funcs)
        ├── fs_alloc.c         # Block 할당 (2 funcs)
        ├── fs_file.c          # 파일 생성 (2 funcs)
        └── fs_ops.c           # 파일 I/O (5 funcs)
```

**42 Norm 준수:**
- ✅ 한 함수당 최대 25줄
- ✅ 한 파일당 최대 5개 함수
- ✅ 한 폴더당 최대 5개 파일
- ✅ 함수 인자 최대 4개
- ✅ norminette 25개 파일 모두 통과

---

## ✨ 주요 기능

### HDD 모드
- ✅ **CHS (Cylinder-Head-Sector) 물리 구조**
  - 16 헤드, 1024 실린더, 512 섹터
  - LBA to CHS 변환
- ✅ **Zone Bit Recording (3 zones)**
  - Outer zone (0-2730): 0.08ms transfer
  - Middle zone (2731-5461): 0.10ms transfer
  - Inner zone (5462-8191): 0.12ms transfer
- ✅ **Read-ahead 캐시 (8 blocks, LRU)**
  - LRU 교체 정책
  - Write-through 무효화
- ✅ **정밀한 비용 모델**
  - Seek cost: |prev_cyl - curr_cyl| × 0.05ms
  - Head switch: 0.05ms per head
  - Rotational latency: 2.0ms (평균)
  - Transfer time: zone별 차등 적용

### SSD 모드
- ✅ **Flash Translation Layer (FTL)**
  - 동적 LBA→PPA 매핑
  - 8192 페이지 주소 공간
- ✅ **Over-Provisioning (10%)**
  - User pages: 7372
  - OP pages: 820
- ✅ **Garbage Collection (Greedy)**
  - 트리거: free pages < 82
  - Victim 선택: max invalid pages
  - Valid page 마이그레이션
- ✅ **Write Amplification 추적**
  - host_writes / nand_writes
  - GC moves 카운팅
  - Erase count 추적

### FS_LITE
- ✅ **간단한 파일시스템**
  - 최대 64개 파일
  - 32바이트 파일명
- ✅ **Extent 기반 할당**
  - 파일당 최대 8개 extent
  - Contiguous block 할당
- ✅ **파일 작업**
  - CF: Create File
  - WF: Write File
  - RF: Read File
  - CHK: Checksum (byte sum)

---

## 📊 현재 상태

- [x] 기본 구조 및 검증
- [x] Block Device Interface
- [x] v2 명세 확정
- [x] HDD CHS 모델
- [x] SSD FTL 구현
- [x] Script Parser
- [x] FS_LITE

---

## 🛠️ 개발 환경

- **언어**: C
- **컴파일러**: gcc/clang
- **표준**: C99
- **규칙**: 42 Norm

---

## 📝 라이선스

Educational Project

---

## 🧪 단위 테스트

`make unit-test` 로 HDD 캐시 동작 단위 테스트 실행. 검증 항목:

- 첫 read 는 miss (cache_misses=1)
- 동일 LBA 재read 는 hit (cache_hits=1)
- write 후 read 는 다시 miss (캐시 무효화 확인)
- total_reads/total_writes 카운터 정합성

## 🩹 최근 수정

- **HDD 캐시 hit 비용 모델**: hit 시 전송 시간(약 0.02ms)을 누적해 캐시 효과가 통계에 반영되도록 수정.
- **`hdd_print_stats`**: cache_hits / cache_misses / hit_rate 출력 추가.
