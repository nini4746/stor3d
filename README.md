# stor3D v2
## 현실적인 블록 스토리지 시뮬레이터

HDD와 SSD의 물리적 제약을 정확히 모델링하는 스토리지 시뮬레이터 프로젝트입니다.

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
├── test_stor3d.sh             # 테스트 스크립트
├── docs/                      # 📚 문서
│   ├── specs/                 # 프로젝트 명세서
│   │   ├── v1/                # 원본 버전
│   │   │   ├── en.md
│   │   │   └── ko.md
│   │   └── v2/                # 현재 버전 ⭐
│   │       ├── en.md
│   │       └── ko.md
│   └── guides/                # 구현 가이드
│       └── implementation.md
├── include/                   # 헤더 파일
│   ├── stor3d.h               # 공통 정의
│   ├── hdd.h                  # HDD 구조체 및 함수
│   └── ssd.h                  # SSD 구조체 및 함수
└── src/                       # 소스 코드 (폴더당 최대 5개 파일)
    ├── main.c
    ├── init/                  # 초기화 (3개)
    │   ├── validation.c       # 입력 검증
    │   └── context.c          # Context init/cleanup
    ├── device/                # 블록 디바이스 (1개)
    │   └── block_device.c
    ├── parser/                # 스크립트 파싱 (1개)
    │   └── parser.c
    ├── hdd/                   # HDD 구현 (1개, 확장 예정)
    │   └── hdd_init.c
    ├── ssd/                   # SSD 구현 (1개, 확장 예정)
    │   └── ssd_init.c
    └── filesystem/            # 파일시스템 (예정)
```

**디렉토리 구조 원칙:**
- 한 폴더당 파일 5개 이하 유지
- 기능별로 명확히 분리
- 파일 많아지면 서브디렉토리 추가

---

## ✨ 주요 기능

### HDD 모드
- ✅ CHS (Cylinder-Head-Sector) 물리 구조
- ✅ Zone Bit Recording (3 zones)
- ✅ Read-ahead 캐시 (8 blocks, LRU)
- ✅ 정밀한 비용 모델 (seek, head switch, rotation)

### SSD 모드
- ✅ Flash Translation Layer (FTL)
- ✅ Over-Provisioning (10%)
- ✅ Wear Leveling
- ✅ Garbage Collection (Greedy / Cost-Benefit)
- ✅ TRIM 지원

### FS_LITE
- ✅ 간단한 파일시스템 (최대 64 파일)
- ✅ Extent 기반 할당
- ✅ Overwrite 지원

---

## 📊 현재 상태

- [x] 기본 구조 및 검증
- [x] Block Device Interface
- [x] v2 명세 확정
- [ ] HDD CHS 모델
- [ ] SSD FTL 구현
- [ ] Script Parser
- [ ] FS_LITE

---

## 🛠️ 개발 환경

- **언어**: C
- **컴파일러**: gcc/clang
- **표준**: C99
- **규칙**: 42 Norm

---

## 📝 라이선스

Educational Project
