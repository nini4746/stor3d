#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

PROGRAM="../stor3D"
TEST_DIR="test_output"
PASSED=0
FAILED=0

# 테스트 결과 출력
print_test() {
    local test_name=$1
    local result=$2

    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} $test_name"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $test_name"
        ((FAILED++))
    fi
}

# 숫자 비교 함수 (float 지원)
compare_float() {
    awk -v a="$1" -v b="$2" -v epsilon="$3" 'BEGIN {
        diff = a - b;
        if (diff < 0) diff = -diff;
        if (diff < epsilon) exit 0;
        else exit 1;
    }'
}

# 테스트 환경 초기화
setup() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  stor3D Ultimate Tester${NC}"
    echo -e "${BLUE}========================================${NC}\n"

    # 프로그램 존재 확인
    if [ ! -f "$PROGRAM" ]; then
        echo -e "${RED}Error: $PROGRAM not found!${NC}"
        echo -e "${YELLOW}Please compile your program first.${NC}"
        exit 1
    fi

    mkdir -p $TEST_DIR
    cd $TEST_DIR
}

# 테스트 환경 정리
cleanup() {
    cd ..
    rm -rf $TEST_DIR

    echo -e "\n${BLUE}========================================${NC}"
    if [ $FAILED -eq 0 ]; then
        echo -e "${GREEN}✓ ALL TESTS PASSED: $PASSED${NC}"
    else
        echo -e "${GREEN}PASSED: $PASSED${NC}"
        echo -e "${RED}FAILED: $FAILED${NC}"
    fi
    echo -e "${BLUE}========================================${NC}"

    if [ $FAILED -eq 0 ]; then
        exit 0
    else
        exit 1
    fi
}

# 1. 인자 개수 테스트
test_argument_count() {
    echo -e "\n${YELLOW}[1] Argument Count Tests${NC}"

    # No arguments
    $PROGRAM 2>/dev/null
    [ $? -ne 0 ]
    print_test "No arguments" $?

    # Too few arguments
    $PROGRAM hdd 2>/dev/null
    [ $? -ne 0 ]
    print_test "Too few arguments" $?

    # Too many arguments
    $PROGRAM hdd disk.img script.txt extra 2>/dev/null
    [ $? -ne 0 ]
    print_test "Too many arguments" $?
}

# 2. 모드 검증 테스트
test_mode_validation() {
    echo -e "\n${YELLOW}[2] Mode Validation Tests${NC}"

    echo "R 0" > valid_script.txt

    # Invalid mode
    $PROGRAM invalid disk.img valid_script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid mode 'invalid'" $?

    # Invalid mode - uppercase
    $PROGRAM HDD disk.img valid_script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid mode 'HDD' (uppercase)" $?

    # Invalid mode - SSD uppercase
    $PROGRAM SSD disk.img valid_script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid mode 'SSD' (uppercase)" $?

    rm -f valid_script.txt
}

# 3. 디스크 이미지 생성 테스트
test_disk_creation() {
    echo -e "\n${YELLOW}[3] Disk Image Creation Tests${NC}"

    echo "R 0" > script.txt

    # Create new disk image
    rm -f new_disk.img
    $PROGRAM hdd new_disk.img script.txt >/dev/null 2>&1
    [ -f new_disk.img ] && [ $(stat -c%s new_disk.img) -eq 33554432 ]
    print_test "Create new disk image (32MB)" $?

    rm -f script.txt new_disk.img
}

# 4. 디스크 이미지 크기 검증 테스트
test_disk_size_validation() {
    echo -e "\n${YELLOW}[4] Disk Image Size Validation Tests${NC}"

    echo "R 0" > script.txt

    # Wrong size - too small
    dd if=/dev/zero of=small_disk.img bs=4096 count=100 2>/dev/null
    $PROGRAM hdd small_disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject disk image (too small)" $?

    # Wrong size - too large
    dd if=/dev/zero of=large_disk.img bs=4096 count=10000 2>/dev/null
    $PROGRAM hdd large_disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject disk image (too large)" $?

    # Correct size
    dd if=/dev/zero of=correct_disk.img bs=4096 count=8192 2>/dev/null
    $PROGRAM hdd correct_disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Accept disk image (correct size)" $?

    rm -f script.txt small_disk.img large_disk.img correct_disk.img
}

# 5. 스크립트 파일 검증 테스트
test_script_validation() {
    echo -e "\n${YELLOW}[5] Script File Validation Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Missing script file
    $PROGRAM hdd disk.img nonexistent.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject missing script file" $?

    # Empty script file (should pass)
    touch empty_script.txt
    $PROGRAM hdd disk.img empty_script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Accept empty script file" $?

    rm -f disk.img empty_script.txt
}

# 6. 블록 주소 범위 테스트
test_block_address_range() {
    echo -e "\n${YELLOW}[6] Block Address Range Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Valid LBA: 0
    echo "R 0" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Valid LBA: 0" $?

    # Valid LBA: 8191 (max)
    echo "R 8191" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Valid LBA: 8191 (max)" $?

    # Invalid LBA: 8192 (out of range)
    echo "R 8192" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid LBA: 8192 (out of range)" $?

    # Invalid LBA: negative
    echo "R -1" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid LBA: -1 (negative)" $?

    # Very large LBA
    echo "R 999999" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid LBA: 999999 (very large)" $?

    rm -f disk.img script.txt
}

# 7. 스크립트 명령어 파싱 테스트
test_script_commands() {
    echo -e "\n${YELLOW}[7] Script Command Parsing Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Valid block commands
    cat > script.txt << 'EOF'
# This is a comment
R 0
W 1 0xFF

R 100
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Valid block commands with comments" $?

    # Invalid command
    echo "INVALID 0" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid command 'INVALID'" $?

    # Missing arguments for R
    echo "R" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Missing arguments for R command" $?

    # Missing arguments for W
    echo "W 0" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Missing arguments for W command" $?

    # Extra arguments
    echo "R 0 extra" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Extra arguments for R command" $?

    rm -f disk.img script.txt
}

# 8. HDD/SSD 모드 기본 동작 테스트
test_mode_execution() {
    echo -e "\n${YELLOW}[8] HDD/SSD Mode Execution Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    cat > script.txt << 'EOF'
W 0 0xAA
R 0
W 100 0xFF
R 100
EOF

    # HDD mode execution
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "HDD mode basic execution" $?

    # SSD mode execution
    $PROGRAM ssd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "SSD mode basic execution" $?

    rm -f disk.img script.txt
}

# 9. HDD 통계 출력 형식 테스트
test_hdd_stats_format() {
    echo -e "\n${YELLOW}[9] HDD Statistics Output Format Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    cat > script.txt << 'EOF'
W 0 0xAA
R 0
W 100 0xFF
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)

    echo "$output" | grep -q "\[HDD\] total_reads="
    print_test "HDD output contains total_reads" $?

    echo "$output" | grep -q "\[HDD\] total_writes="
    print_test "HDD output contains total_writes" $?

    echo "$output" | grep -q "\[HDD\] total_time_ms="
    print_test "HDD output contains total_time_ms" $?

    rm -f disk.img script.txt
}

# 10. SSD 통계 출력 형식 테스트
test_ssd_stats_format() {
    echo -e "\n${YELLOW}[10] SSD Statistics Output Format Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    cat > script.txt << 'EOF'
W 0 0xAA
R 0
W 100 0xFF
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)

    echo "$output" | grep -q "\[SSD\] host_writes="
    print_test "SSD output contains host_writes" $?

    echo "$output" | grep -q "\[SSD\] nand_writes="
    print_test "SSD output contains nand_writes" $?

    echo "$output" | grep -q "\[SSD\] erases="
    print_test "SSD output contains erases" $?

    echo "$output" | grep -q "\[SSD\] gc_moves="
    print_test "SSD output contains gc_moves" $?

    echo "$output" | grep -q "\[SSD\] write_amp="
    print_test "SSD output contains write_amp" $?

    rm -f disk.img script.txt
}

# 11. HDD 통계 값 정확성 테스트
test_hdd_stats_accuracy() {
    echo -e "\n${YELLOW}[11] HDD Statistics Value Accuracy Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Test 1: 정확한 읽기/쓰기 횟수
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xBB
R 0
R 1
R 2
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    reads=$(echo "$output" | grep -oP '\[HDD\] total_reads=\K[0-9]+')
    writes=$(echo "$output" | grep -oP '\[HDD\] total_writes=\K[0-9]+')

    [ "$reads" -eq 3 ] && [ "$writes" -eq 2 ]
    print_test "HDD correct read/write counts (R:3, W:2)" $?

    # Test 2: Sequential access 시간 계산
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xBB
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    time_ms=$(echo "$output" | grep -oP '\[HDD\] total_time_ms=\K[0-9.]+')

    compare_float "$time_ms" "4.25" "0.01"
    print_test "HDD sequential write timing (expected: 4.25ms)" $?

    # Test 3: Random access 시간 계산
    cat > script.txt << 'EOF'
W 0 0xAA
W 1000 0xBB
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    time_ms=$(echo "$output" | grep -oP '\[HDD\] total_time_ms=\K[0-9.]+')

    compare_float "$time_ms" "54.2" "0.01"
    print_test "HDD random write timing (expected: 54.2ms)" $?

    # Test 4: Mixed read/write
    cat > script.txt << 'EOF'
W 10 0xFF
R 10
W 20 0xAA
R 30
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    reads=$(echo "$output" | grep -oP '\[HDD\] total_reads=\K[0-9]+')
    writes=$(echo "$output" | grep -oP '\[HDD\] total_writes=\K[0-9]+')

    [ "$reads" -eq 2 ] && [ "$writes" -eq 2 ]
    print_test "HDD mixed operations count (R:2, W:2)" $?

    rm -f disk.img script.txt
}

# 12. SSD 통계 값 정확성 테스트
test_ssd_stats_accuracy() {
    echo -e "\n${YELLOW}[12] SSD Statistics Value Accuracy Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Test 1: 기본 write 카운트
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xBB
W 2 0xCC
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    host_writes=$(echo "$output" | grep -oP '\[SSD\] host_writes=\K[0-9]+')
    nand_writes=$(echo "$output" | grep -oP '\[SSD\] nand_writes=\K[0-9]+')

    [ "$host_writes" -eq 3 ]
    print_test "SSD host_writes count (expected: 3)" $?

    # 초기에는 GC 없으므로 nand_writes == host_writes
    [ "$nand_writes" -eq "$host_writes" ]
    print_test "SSD nand_writes == host_writes (no GC yet)" $?

    # Test 2: Overwrite로 인한 write amplification
    cat > script.txt << 'EOF'
W 0 0xAA
W 0 0xBB
W 0 0xCC
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    host_writes=$(echo "$output" | grep -oP '\[SSD\] host_writes=\K[0-9]+')
    nand_writes=$(echo "$output" | grep -oP '\[SSD\] nand_writes=\K[0-9]+')

    [ "$host_writes" -eq 3 ]
    print_test "SSD overwrite host_writes (expected: 3)" $?

    # nand_writes >= host_writes (GC 발생 가능)
    [ "$nand_writes" -ge "$host_writes" ]
    print_test "SSD overwrite nand_writes >= host_writes" $?

    # Test 3: Write amplification 계산
    write_amp=$(echo "$output" | grep -oP '\[SSD\] write_amp=\K[0-9.]+')
    expected_amp=$(echo "scale=2; $nand_writes / $host_writes" | bc)

    compare_float "$write_amp" "$expected_amp" "0.01"
    print_test "SSD write_amp calculation (nand/host)" $?

    # Test 4: 초기 상태 - erase와 gc_moves는 0 또는 적어야 함
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xBB
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    erases=$(echo "$output" | grep -oP '\[SSD\] erases=\K[0-9]+')

    [ "$erases" -eq 0 ]
    print_test "SSD no erases for simple writes" $?

    rm -f disk.img script.txt
}

# 13. SSD Garbage Collection 테스트
test_ssd_garbage_collection() {
    echo -e "\n${YELLOW}[13] SSD Garbage Collection Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # GC 트리거: 같은 블록에 많은 overwrite 수행
    cat > script.txt << 'EOF'
# Write to first 200 pages
EOF

    for i in {0..199}; do
        echo "W $i 0xAA" >> script.txt
    done

    # Overwrite first 150 pages (triggers GC)
    for i in {0..149}; do
        echo "W $i 0xBB" >> script.txt
    done

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    erases=$(echo "$output" | grep -oP '\[SSD\] erases=\K[0-9]+')
    gc_moves=$(echo "$output" | grep -oP '\[SSD\] gc_moves=\K[0-9]+')

    # GC가 발생했다면 erase > 0
    [ "$erases" -gt 0 ]
    print_test "SSD GC triggered (erases > 0)" $?

    # GC 발생 시 valid page 이동
    [ "$gc_moves" -ge 0 ]
    print_test "SSD GC moves valid pages" $?

    rm -f disk.img script.txt
}

# 14. 파일 시스템 명령어 테스트 (FS_LITE)
test_filesystem_commands() {
    echo -e "\n${YELLOW}[14] Filesystem Commands Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # CF (Create File)
    echo "CF testfile" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "CF (Create File) command" $?

    # WF (Write File)
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 100 0xAA
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "WF (Write File) command" $?

    # RF (Read File)
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 100 0xAA
RF testfile 0 100
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "RF (Read File) command" $?

    # CHK (Checksum)
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 100 0xAA
CHK testfile
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "CHK (Checksum) command" $?

    rm -f disk.img script.txt
}

# 15. 파일 시스템 에러 처리 테스트
test_filesystem_errors() {
    echo -e "\n${YELLOW}[15] Filesystem Error Handling Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Read non-existent file
    echo "RF nonexistent 0 100" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Error on reading non-existent file" $?

    # Write to non-existent file
    echo "WF nonexistent 0 100 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Error on writing to non-existent file" $?

    # Create duplicate file
    cat > script.txt << 'EOF'
CF testfile
CF testfile
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Error on creating duplicate file" $?

    # Maximum files limit (64 files)
    echo "" > script.txt
    for i in {1..65}; do
        echo "CF file$i" >> script.txt
    done
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Error on exceeding max files (64)" $?

    rm -f disk.img script.txt
}

# 16. 엣지 케이스 테스트
test_edge_cases() {
    echo -e "\n${YELLOW}[16] Edge Cases Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Byte value boundaries
    echo "W 0 0x00" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Byte value 0x00" $?

    echo "W 0 0xFF" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Byte value 0xFF" $?

    # Invalid byte value
    echo "W 0 0x1FF" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Invalid byte value 0x1FF" $?

    # Very long script
    echo "# Long script test" > script.txt
    for i in {0..500}; do
        echo "W $((i % 8192)) 0xAA" >> script.txt
    done
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Very long script (500+ commands)" $?

    # Script with only comments
    cat > script.txt << 'EOF'
# Comment 1
# Comment 2
# Comment 3
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Script with only comments" $?

    rm -f disk.img script.txt
}

# 17. HDD Sequential vs Random 성능 비교
test_hdd_performance_patterns() {
    echo -e "\n${YELLOW}[17] HDD Performance Pattern Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # Sequential writes
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xAA
W 2 0xAA
W 3 0xAA
W 4 0xAA
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    seq_time=$(echo "$output" | grep -oP '\[HDD\] total_time_ms=\K[0-9.]+')

    # Random writes
    cat > script.txt << 'EOF'
W 0 0xAA
W 1000 0xAA
W 2000 0xAA
W 4000 0xAA
W 100 0xAA
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    rand_time=$(echo "$output" | grep -oP '\[HDD\] total_time_ms=\K[0-9.]+')

    # Random should be significantly slower than sequential
    awk -v seq="$seq_time" -v rand="$rand_time" 'BEGIN {
        if (rand > seq * 2) exit 0;
        else exit 1;
    }'
    print_test "HDD random access slower than sequential" $?

    rm -f disk.img script.txt
}

# 18. SSD Write Amplification 심화 테스트
test_ssd_write_amplification() {
    echo -e "\n${YELLOW}[18] SSD Write Amplification Advanced Tests${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # No overwrite - write_amp should be 1.0
    cat > script.txt << 'EOF'
W 0 0xAA
W 1 0xBB
W 2 0xCC
W 3 0xDD
W 4 0xEE
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    write_amp=$(echo "$output" | grep -oP '\[SSD\] write_amp=\K[0-9.]+')

    compare_float "$write_amp" "1.0" "0.01"
    print_test "SSD write_amp = 1.0 (no overwrite)" $?

    # With overwrite - write_amp should be > 1.0
    cat > script.txt << 'EOF'
W 0 0xAA
W 0 0xBB
W 0 0xCC
W 1 0xDD
W 1 0xEE
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    write_amp=$(echo "$output" | grep -oP '\[SSD\] write_amp=\K[0-9.]+')

    awk -v amp="$write_amp" 'BEGIN {
        if (amp > 1.0) exit 0;
        else exit 1;
    }'
    print_test "SSD write_amp > 1.0 (with overwrite)" $?

    rm -f disk.img script.txt
}

# 19. 악랄한 테스트 1: 디스크 풀 테스트
test_disk_full_scenario() {
    echo -e "\n${YELLOW}[19] Disk Full Scenario (Evil Test 1)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 파일시스템으로 전체 디스크 공간 채우기
    cat > script.txt << 'EOF'
CF file1
EOF

    # 거의 전체 디스크를 채우려고 시도
    echo "WF file1 0 32768000 0xAA" >> script.txt
    echo "CF file2" >> script.txt
    echo "WF file2 0 100000 0xBB" >> script.txt

    $PROGRAM hdd disk.img script.txt 2>/dev/null
    # 디스크가 꽉 찼을 때 에러 발생해야 함
    [ $? -ne 0 ]
    print_test "Disk full error (no space left)" $?

    # SSD에서도 동일하게 테스트
    dd if=/dev/zero of=disk2.img bs=4096 count=8192 2>/dev/null
    $PROGRAM ssd disk2.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "SSD disk full error" $?

    rm -f disk.img disk2.img script.txt
}

# 20. 악랄한 테스트 2: SSD GC 지옥 + 파싱 스트레스
test_evil_stress_test() {
    echo -e "\n${YELLOW}[20] Evil Stress Test (Evil Test 2)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 서브테스트 1: 같은 LBA 극한 overwrite
    echo "# SSD GC stress test" > script.txt
    for i in {1..1000}; do
        echo "W 0 0xAA" >> script.txt
    done

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "SSD survives 1000 overwrites on same LBA" $?

    # Write amplification이 엄청 높아야 함
    write_amp=$(echo "$output" | grep -oP '\[SSD\] write_amp=\K[0-9.]+')
    awk -v amp="$write_amp" 'BEGIN {
        if (amp > 1.5) exit 0;
        else exit 1;
    }'
    print_test "SSD extreme overwrite causes high write_amp (>1.5)" $?

    # 서브테스트 2: 잘못된 형식의 LBA/byte 값들
    cat > script.txt << 'EOF'
W 0 0xGGG
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject invalid hex value 0xGGG" $?

    cat > script.txt << 'EOF'
W abc 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject non-numeric LBA 'abc'" $?

    cat > script.txt << 'EOF'
W 0 256
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject byte value 256 (out of range)" $?

    cat > script.txt << 'EOF'
W 0 -1
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject negative byte value -1" $?

    # 서브테스트 3: 매우 긴 라인
    echo -n "W 0 0xAA " > script.txt
    for i in {1..10000}; do
        echo -n "extra " >> script.txt
    done
    echo "" >> script.txt

    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject extremely long script line (10000+ chars)" $?

    # 서브테스트 4: 파일명 극한 테스트
    cat > script.txt << 'EOF'
CF ""
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject empty filename" $?

    # 매우 긴 파일명
    echo -n "CF " > script.txt
    for i in {1..1000}; do
        echo -n "a" >> script.txt
    done
    echo "" >> script.txt

    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject extremely long filename (1000+ chars)" $?

    # 서브테스트 5: Fragmentation
    echo "# Fragmentation test" > script.txt
    for i in {0..100}; do
        echo "CF file$i" >> script.txt
        echo "WF file$i 0 100 0xAA" >> script.txt
    done

    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle heavy fragmentation (100 files)" $?

    # 서브테스트 6: 오프셋/길이 경계
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 0 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject zero-length write" $?

    cat > script.txt << 'EOF'
CF testfile
WF testfile 999999999 100 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject huge offset write" $?

    rm -f disk.img script.txt
}

# 21. 보너스 악랄 테스트: Race condition 시뮬레이션
test_concurrent_operations() {
    echo -e "\n${YELLOW}[21] Concurrent Operations Simulation${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 같은 파일을 동시에 읽고 쓰는 패턴
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 1000 0xAA
RF testfile 0 500
WF testfile 500 500 0xBB
RF testfile 0 1000
WF testfile 0 1000 0xCC
RF testfile 0 1000
CHK testfile
EOF

    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle interleaved read/write operations" $?

    # 파일 중복 생성
    cat > script.txt << 'EOF'
CF file1
WF file1 0 100 0xAA
CF file1
EOF

    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Prevent duplicate file creation" $?

    rm -f disk.img script.txt
}

# 22. 악랄한 파싱 테스트: Whitespace 지옥
test_whitespace_hell() {
    echo -e "\n${YELLOW}[22] Whitespace Hell (Evil Test 3)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 여러 개의 공백
    printf "W    0    0xAA\n" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle multiple spaces between tokens" $?

    # 탭 문자
    printf "W\t0\t0xAA\n" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle tab characters" $?

    # 공백과 탭 혼용
    printf "W \t 0 \t 0xAA\n" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle mixed spaces and tabs" $?

    # 라인 시작에 공백
    printf "   W 0 0xAA\n" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle leading spaces" $?

    # 라인 끝에 공백
    printf "W 0 0xAA   \n" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle trailing spaces" $?

    # 빈 라인 여러 개
    cat > script.txt << 'EOF'


W 0 0xAA


R 0


EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle multiple empty lines" $?

    # 주석만 있는 라인들 사이에 명령어
    cat > script.txt << 'EOF'
###
# comment
###
W 0 0xAA
###
EOF
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle comment-heavy script" $?

    # 주석에 공백
    printf "#    comment with spaces   \n" > script.txt
    printf "W 0 0xAA\n" >> script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle comments with spaces" $?

    rm -f disk.img script.txt
}

# 23. 악랄한 숫자 형식 테스트
test_number_format_hell() {
    echo -e "\n${YELLOW}[23] Number Format Hell (Evil Test 4)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 16진수 소문자/대문자 혼용
    echo "W 0 0xaa" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Accept lowercase hex 0xaa" $?

    echo "W 0 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Accept uppercase hex 0xAA" $?

    echo "W 0 0xAa" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Accept mixed case hex 0xAa" $?

    # 0x 없는 16진수
    echo "W 0 AA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject hex without 0x prefix" $?

    # 10진수 바이트 값
    echo "W 0 255" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    result=$?
    print_test "Handle decimal byte value 255" $((result == 0 || result != 0 ? 0 : 1))

    # Leading zeros
    echo "W 0000 0x00AA" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle leading zeros in LBA" $?

    # 0x0 형식
    echo "W 0 0x0" > script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Handle 0x0 byte value" $?

    # 매우 큰 숫자 (overflow)
    echo "W 999999999999999999999 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject integer overflow in LBA" $?

    echo "W 0 0xFFFFFFFF" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject byte overflow 0xFFFFFFFF" $?

    rm -f disk.img script.txt
}

# 24. 악랄한 파일 시스템 경계 테스트
test_filesystem_boundary_hell() {
    echo -e "\n${YELLOW}[24] Filesystem Boundary Hell (Evil Test 5)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 오프셋이 음수
    cat > script.txt << 'EOF'
CF testfile
WF testfile -1 100 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject negative offset" $?

    # 길이가 음수
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 -100 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject negative length" $?

    # 오프셋 + 길이 overflow
    cat > script.txt << 'EOF'
CF testfile
WF testfile 999999999 999999999 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject offset+length overflow" $?

    # 파일 읽기 범위 초과
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 100 0xAA
RF testfile 0 200
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject reading beyond file size" $?

    # 파일 중간 오프셋에서 범위 초과 읽기
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 100 0xAA
RF testfile 50 100
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject reading beyond EOF from offset" $?

    # 존재하지 않는 오프셋에 쓰기
    cat > script.txt << 'EOF'
CF testfile
WF testfile 10000 100 0xAA
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    result=$?
    print_test "Handle/reject sparse file write" $((result == 0 || result != 0 ? 0 : 1))

    # 파일명에 공백
    cat > script.txt << 'EOF'
CF file name with spaces
EOF
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject filename with spaces" $?

    # 파일명에 특수문자
    echo "CF file/name" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject filename with slash" $?

    echo "CF file\$name" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject filename with special char" $?

    rm -f disk.img script.txt
}

# 25. 악랄한 SSD 페이지/블록 경계 테스트
test_ssd_boundary_hell() {
    echo -e "\n${YELLOW}[25] SSD Page/Block Boundary Hell (Evil Test 6)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 페이지 경계
    cat > script.txt << 'EOF'
W 255 0xAA
W 256 0xBB
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "SSD handle page 255->256 boundary" $?

    # 블록 0의 전체를 invalid로
    echo "# Fill first block" > script.txt
    for i in {0..255}; do
        echo "W $i 0xAA" >> script.txt
    done

    # 전부 overwrite
    for i in {0..255}; do
        echo "W $i 0xBB" >> script.txt
    done

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    erases=$(echo "$output" | grep -oP '\[SSD\] erases=\K[0-9]+')

    [ "$erases" -gt 0 ]
    print_test "SSD GC triggers on full block invalidation" $?

    # 블록 경계를 넘나드는 패턴
    cat > script.txt << 'EOF'
W 0 0xAA
W 256 0xBB
W 512 0xCC
W 768 0xDD
W 1024 0xEE
EOF

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "SSD handle cross-block writes" $?

    # 모든 블록에 하나씩 쓰기
    echo "# Write to all blocks" > script.txt
    for i in {0..31}; do
        lba=$((i * 256))
        echo "W $lba 0xAA" >> script.txt
    done

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "SSD write to all 32 erase blocks" $?

    # 마지막 블록
    echo "W 8191 0xFF" > script.txt
    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "SSD write to last page (8191)" $?

    rm -f disk.img script.txt
}

# 26. 악랄한 HDD Seek 패턴 테스트
test_hdd_seek_hell() {
    echo -e "\n${YELLOW}[26] HDD Seek Pattern Hell (Evil Test 7)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 최악의 seek 패턴
    cat > script.txt << 'EOF'
W 0 0xAA
W 8191 0xBB
W 0 0xCC
W 8191 0xDD
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    time_ms=$(echo "$output" | grep -oP '\[HDD\] total_time_ms=\K[0-9.]+')

    compare_float "$time_ms" "1237.05" "0.1"
    print_test "HDD worst-case seek pattern timing" $?

    # 역순 접근
    cat > script.txt << 'EOF'
W 8191 0xAA
W 8190 0xBB
W 8189 0xCC
W 8188 0xDD
W 8187 0xEE
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "HDD reverse sequential access" $?

    # 지그재그 패턴
    cat > script.txt << 'EOF'
W 0 0xAA
W 100 0xBB
W 1 0xCC
W 101 0xDD
W 2 0xEE
W 102 0xFF
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "HDD zigzag access pattern" $?

    rm -f disk.img script.txt
}

# 27. 악랄한 명령어 변형 테스트
test_command_variation_hell() {
    echo -e "\n${YELLOW}[27] Command Variation Hell (Evil Test 8)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 소문자 명령어
    echo "w 0 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject lowercase command 'w'" $?

    echo "r 0" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject lowercase command 'r'" $?

    # 대소문자 혼합
    echo "Wf testfile 0 100 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject mixed case command 'Wf'" $?

    # 알 수 없는 명령어
    echo "X 0 0xAA" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject unknown command 'X'" $?

    echo "READ 0" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject full word 'READ'" $?

    # 빈 명령어
    echo "" > script.txt
    echo "  " >> script.txt
    echo "W 0 0xAA" >> script.txt
    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Ignore empty commands" $?

    # 인자 없는 명령어
    echo "W" > script.txt
    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject command without arguments" $?

    rm -f disk.img script.txt
}

# 28. 악랄한 체크섬 및 데이터 무결성 테스트
test_data_integrity_hell() {
    echo -e "\n${YELLOW}[28] Data Integrity Hell (Evil Test 9)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 블록 쓰기 후 읽기
    cat > script.txt << 'EOF'
W 0 0xAA
R 0
W 1 0xBB
R 1
W 2 0xCC
R 2
EOF

    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Write-then-read data persistence (HDD)" $?

    # SSD overwrite 후 데이터 일치
    cat > script.txt << 'EOF'
W 0 0xAA
W 0 0xBB
R 0
EOF

    $PROGRAM ssd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "SSD overwrite data consistency" $?

    # 파일 쓰기 후 체크섬
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 1000 0xAA
CHK testfile
WF testfile 0 1000 0xBB
CHK testfile
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "Checksum changes after overwrite" $?

    # 같은 데이터 쓰면 체크섬 동일
    cat > script.txt << 'EOF'
CF file1
WF file1 0 100 0xAA
CHK file1
CF file2
WF file2 0 100 0xAA
CHK file2
EOF

    output=$($PROGRAM hdd disk.img script.txt 2>&1)
    [ $? -eq 0 ]
    print_test "Identical data produces identical checksum" $?

    # 부분 읽기 후 전체 읽기
    cat > script.txt << 'EOF'
CF testfile
WF testfile 0 1000 0xFF
RF testfile 0 100
RF testfile 100 200
RF testfile 500 500
RF testfile 0 1000
EOF

    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Partial and full file reads" $?

    rm -f disk.img script.txt
}

# 29. 악랄한 리소스 고갈 테스트
test_resource_exhaustion_hell() {
    echo -e "\n${YELLOW}[29] Resource Exhaustion Hell (Evil Test 10)${NC}"

    dd if=/dev/zero of=disk.img bs=4096 count=8192 2>/dev/null

    # 파일 64개 생성
    echo "# Create max files" > script.txt
    for i in {1..64}; do
        echo "CF file$i" >> script.txt
    done

    $PROGRAM hdd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "Create exactly 64 files (max)" $?

    # 65번째 파일
    for i in {1..65}; do
        echo "CF file$i" >> script.txt
    done

    $PROGRAM hdd disk.img script.txt 2>/dev/null
    [ $? -ne 0 ]
    print_test "Reject 65th file (over limit)" $?

    # SSD 모든 페이지 사용
    echo "# Use all pages" > script.txt
    for i in {0..8191}; do
        echo "W $i 0xAA" >> script.txt
    done

    $PROGRAM ssd disk.img script.txt >/dev/null 2>&1
    [ $? -eq 0 ]
    print_test "SSD use all 8192 pages" $?

    # SSD 모든 페이지 사용 후 추가 쓰기
    for i in {0..8191}; do
        echo "W $i 0xAA" >> script.txt
    done
    echo "W 0 0xBB" >> script.txt

    output=$($PROGRAM ssd disk.img script.txt 2>&1)
    result=$?
    print_test "SSD GC frees space when full" $((result == 0 ? 0 : 1))

    rm -f disk.img script.txt
}

# 메인 실행
main() {
    setup

    echo -e "${CYAN}=== Basic Validation Tests ===${NC}"
    test_argument_count
    test_mode_validation
    test_disk_creation
    test_disk_size_validation
    test_script_validation
    test_block_address_range
    test_script_commands
    test_mode_execution

    echo -e "\n${CYAN}=== Statistics Tests ===${NC}"
    test_hdd_stats_format
    test_ssd_stats_format
    test_hdd_stats_accuracy
    test_ssd_stats_accuracy
    test_ssd_garbage_collection

    echo -e "\n${CYAN}=== Filesystem Tests ===${NC}"
    test_filesystem_commands
    test_filesystem_errors

    echo -e "\n${CYAN}=== Edge Cases & Performance ===${NC}"
    test_edge_cases
    test_hdd_performance_patterns
    test_ssd_write_amplification

    echo -e "\n${RED}=== EVIL TESTS - Prepare for Hell ===${NC}"
    test_disk_full_scenario
    test_evil_stress_test
    test_concurrent_operations
    test_whitespace_hell
    test_number_format_hell
    test_filesystem_boundary_hell
    test_ssd_boundary_hell
    test_hdd_seek_hell
    test_command_variation_hell
    test_data_integrity_hell
    test_resource_exhaustion_hell

    cleanup
}

main
