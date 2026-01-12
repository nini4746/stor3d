# stor3D
## HDD / SSD 의미를 반영한 블록 스토리지 시뮬레이터

---

## I. 서문 (Foreword)

현대 운영체제와 데이터베이스는 저장 장치를 파일이 아닌 **블록 디바이스(Block Device)**로 다룹니다.  
이 프로젝트의 목적은 HDD나 SSD를 물리적으로 재구현하는 것이 아니라,  
**이 저장 장치들이 소프트웨어에 강제하는 제약을 코드로 구현하고 체험하는 것**입니다.

본 과제에서는 다음 질문들에 대해 **이론 설명이 아닌, 코드와 실행 결과로 답해야 합니다.**

- 왜 저장 장치는 바이트 단위가 아니라 블록 단위로 추상화되는가?
- 왜 HDD에서는 순차 접근과 랜덤 접근이 본질적으로 다른가?
- 왜 SSD의 overwrite는 단순한 제자리 덮어쓰기가 아닌가?
- 왜 Garbage Collection과 Write Amplification은 피할 수 없는가?

---

## II. 목표 (Goals)

- 블록 디바이스 추상화 이해
- HDD 접근 비용 모델 시뮬레이션
- SSD 플래시 메모리 제약(Page / Block / Erase) 구현
- 저장 장치 특성이 상위 계층(File API)에 미치는 영향 관찰

---

## III. 일반 규칙 (General Instructions)

- 사용 언어: **C**
- Norm 규칙을 반드시 준수해야 합니다.
- 전역 변수 사용은 **금지**됩니다.
- 메모리 누수는 허용되지 않습니다.
- segmentation fault, double free, undefined behavior 발생 시 **실패 처리**됩니다.
- 디버그 출력은 제출 시 반드시 제거되어야 합니다.
- Makefile은 다음 규칙을 포함해야 합니다:
  - `all`, `clean`, `fclean`, `re`
- 불필요한 relinking은 허용되지 않습니다.

---

## IV. 허용 함수 (Authorized Functions)

### 기본 허용 함수

- 파일 I/O  
  `open`, `close`, `read`, `write`
- 메모리  
  `malloc`, `free`
- 시간  
  `gettimeofday`
- 에러 처리  
  `perror`, `strerror`
- 종료  
  `exit`

### 추가 허용 함수 (문자열 / 메모리 / 파싱 최소)

아래 함수만 추가로 허용됩니다.

- `strlen`
- `strnlen`
- `strncmp`
- `strchr`
- `memset`
- `memcpy`
- `memcmp`
- `isdigit` (또는 `<ctype.h>`의 `isdigit`)
- `strtol`

> 위 목록에 없는 모든 libc 함수는 **엄격히 금지**됩니다.  

---

## V. 프로그램 이름 및 실행 방법

### 프로그램 이름
```
stor3D
```

### 실행 방법
```
./stor3D <mode> <disk.img> <script.txt>
```

- `<mode>` : `hdd` 또는 `ssd`
- `<disk.img>` : 디스크 이미지 파일
- `<script.txt>` : 워크로드 스크립트 파일

---

## VI. 디스크 이미지 규격 (Disk Image Specification)

- 블록 크기 : **4096 bytes**
- 블록 개수 : **8192**
- 전체 크기 : **32 MB**

### 규칙
- `disk.img`가 존재하지 않으면 새로 생성해야 합니다.
- 파일이 존재하지만 크기가 정확히 일치하지 않으면, 다음을 출력하고 종료해야 합니다.

```
Error
invalid disk image size
```

---

## VII. 블록 디바이스 인터페이스 (필수)

```c
int read_block(size_t lba, void *buf);
int write_block(size_t lba, const void *buf);
```

### 제약 사항
- `lba`는 `[0, 8192)` 범위 내여야 합니다.
- 부분 블록 접근은 허용되지 않습니다.

---

## VIII. 스크립트 규격 (Script Specification)

### 블록 명령
- `R <lba>`
- `W <lba> <byte>`

### 파일 명령 (FS_LITE)
- `CF <name>`
- `WF <name> <offset> <len> <byte>`
- `RF <name> <offset> <len>`
- `CHK <name>`

### 파싱 규칙
- 토큰은 공백으로 구분됩니다.
- 빈 줄 및 `#`로 시작하는 줄은 무시됩니다.
- 한 줄이라도 규격을 위반하면 즉시 에러 처리합니다.

---

## IX. HDD 모드 요구사항

### 비용 모델

- Seek 비용 : `abs(prev_lba - curr_lba) * 0.05 ms`
- 회전 지연 : `2.0 ms`
- 전송 비용 : `0.1 ms`

### 필수 출력
```
[HDD] total_reads=...
[HDD] total_writes=...
[HDD] total_time_ms=...
```

---

## X. SSD 모드 요구사항

### 플래시 모델

- Page 크기 : 4096 bytes
- Erase Block 당 Page 수 : 256
- Erase Block 개수 : 32

### 강제 제약

- overwrite는 허용되지 않습니다.
- 모든 write는 새로운 page에 기록됩니다.
- 기존 page는 invalid 처리됩니다.
- erase는 block 단위로만 가능합니다.

### Garbage Collection

- invalid page 비율이 **50% 이상**인 block이 GC 후보
- invalid 비율이 가장 높은 block 1개 선택
- valid page 복사 후 erase 수행

### 필수 출력
```
[SSD] host_writes=...
[SSD] nand_writes=...
[SSD] erases=...
[SSD] gc_moves=...
[SSD] write_amp=...
```

---

## XI. FS_LITE 요구사항

- 최대 파일 수 : **64**
- 메타데이터는 고정 테이블 사용
- 데이터는 extent 리스트로 관리
- free block 관리 필수
- overwrite 동작이 가능해야 함

---

## XII. 에러 처리 규칙

```
Error
<명확한 에러 메시지>
```

---

## XIII. 금지 사항

- 디스크를 단순 배열처럼 취급하는 구현
- SSD overwrite 허용
- HDD/SSD 로직을 상위 계층에서 직접 분기
- 허용되지 않은 함수 사용
- 파싱 에러 무시

---

## XIV. 보너스 (선택)

- TRIM / TRIMF
- HDD read-ahead 캐시
- 워크로드 자동 생성기

---

## XV. 마무리

이 과제는 저장 장치를 구현하는 과제가 아니라,  
**저장 장치의 제약이 소프트웨어 설계를 어떻게 결정하는지 이해했는지를 평가하는 과제**입니다.
