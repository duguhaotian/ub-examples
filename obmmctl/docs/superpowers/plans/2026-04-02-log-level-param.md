# Log Level Parameter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Support `-v/-vv` verbose level control in obmmctl CLI for INFO and DEBUG logging.

**Architecture:** Replace `bool verbose` with `int verbose_count` counter. Each `-v` increments counter; commands set log level based on count (0=ERROR, 1=INFO, 2+=DEBUG).

**Tech Stack:** C11, getopt_long, existing log.h API

---

## File Structure

| File | Responsibility |
|------|----------------|
| `include/cli_parser.h` | `cmd_options_t` struct definition |
| `src/cli_parser.c` | CLI parsing, initialization, help text |
| `src/cmd_lend.c` | Lend command log level setup |
| `src/cmd_borrow.c` | Borrow command log level setup |
| `src/cmd_status.c` | Status command log level setup |
| `src/cmd_owner.c` | Owner command log level setup |
| `tests/test_cli_parser.c` | Parser unit tests |

---

### Task 1: Update cli_parser.h struct

**Files:**
- Modify: `obmmctl/include/cli_parser.h:38`

- [ ] **Step 1: Change verbose field to verbose_count**

```c
// In cmd_options_t struct, line 38
// Change from:
    bool verbose;           /* Verbose output */
// To:
    int verbose_count;      /* Verbose level: 0=ERROR, 1=INFO, 2+=DEBUG */
```

- [ ] **Step 2: Verify build compiles**

Run: `cd /home/ubuntu/workspace/github/ub-examples/obmmctl && gcc -Wall -Wextra -Werror -std=c11 -I./include -c src/cli_parser.c -o build/cli_parser.o 2>&1`
Expected: Compilation errors (expected - tests not updated yet)

---

### Task 2: Update cli_parser.c implementation

**Files:**
- Modify: `obmmctl/src/cli_parser.c:37` (initialization)
- Modify: `obmmctl/src/cli_parser.c:150` (help text)
- Modify: `obmmctl/src/cli_parser.c:331` (parsing)

- [ ] **Step 1: Update initialization**

```c
// In cmd_options_init(), line 37
// Change from:
        opts->verbose = false;
// To:
        opts->verbose_count = 0;
```

- [ ] **Step 2: Update help text**

```c
// In print_general_usage(), line 150
// Change from:
    printf("  -v, --verbose   Verbose output\n");
// To:
    printf("  -v, --verbose   Verbose output (repeat for more: -v=INFO, -vv=DEBUG)\n");
```

- [ ] **Step 3: Update parsing logic**

```c
// In parse_args(), case 'v' handler, line 330-332
// Change from:
            case 'v':
                opts->verbose = true;
                break;
// To:
            case 'v':
                opts->verbose_count++;
                break;
```

---

### Task 3: Update test_cli_parser.c

**Files:**
- Modify: `obmmctl/tests/test_cli_parser.c:40` (init test)
- Add: New test for `-v` counting

- [ ] **Step 1: Update init test**

```c
// Line 40, change from:
        TEST_ASSERT(opts.verbose == false, "verbose should be false after init");
// To:
        TEST_ASSERT(opts.verbose_count == 0, "verbose_count should be 0 after init");
```

- [ ] **Step 2: Add verbose count test**

Add new test after the existing parse_args test block (around line 100):

```c
void test_verbose_counting(void)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    // Test single -v
    char *args1[] = { "obmmctl", "lend", "-v", "--eid", "/sys/devices/ub_bus_controller0/0" };
    int ret = parse_args(5, args1, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with -v should succeed");
    TEST_ASSERT(opts.verbose_count == 1, "single -v should set verbose_count to 1");

    // Test double -vv
    cmd_options_init(&opts);
    char *args2[] = { "obmmctl", "lend", "-v", "-v", "--eid", "/sys/devices/ub_bus_controller0/0" };
    ret = parse_args(6, args2, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with -vv should succeed");
    TEST_ASSERT(opts.verbose_count == 2, "-vv should set verbose_count to 2");

    // Test --verbose --verbose
    cmd_options_init(&opts);
    char *args3[] = { "obmmctl", "lend", "--verbose", "--verbose", "--eid", "/sys/devices/ub_bus_controller0/0" };
    ret = parse_args(6, args3, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with --verbose --verbose should succeed");
    TEST_ASSERT(opts.verbose_count == 2, "double --verbose should set verbose_count to 2");
}
```

- [ ] **Step 3: Register new test in test_runner**

In `tests/test_runner.c`, add the test call in the main function:

```c
// Add after existing test_cli_parser() call:
    test_verbose_counting();
    printf("verbose counting tests: passed\n");
```

- [ ] **Step 4: Run tests to verify**

Run: `cd /home/ubuntu/workspace/github/ub-examples/obmmctl && make test 2>&1`
Expected: All tests pass including verbose_counting

---

### Task 4: Update cmd_lend.c log level setting

**Files:**
- Modify: `obmmctl/src/cmd_lend.c:37-39`

- [ ] **Step 1: Update log level logic**

```c
// Lines 37-39, change from:
    /* Set verbose mode */
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    /* Set log level based on verbose count */
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

- [ ] **Step 2: Update unlend verbose handling**

```c
// Lines 96-98, change from:
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

---

### Task 5: Update cmd_borrow.c log level setting

**Files:**
- Modify: `obmmctl/src/cmd_borrow.c:49-51`
- Modify: `obmmctl/src/cmd_borrow.c:106-108`

- [ ] **Step 1: Update borrow log level logic**

```c
// Lines 49-51, change from:
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

- [ ] **Step 2: Update unborrow log level logic**

```c
// Lines 106-108, change from:
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

---

### Task 6: Update cmd_status.c log level setting

**Files:**
- Modify: `obmmctl/src/cmd_status.c:29-31`

- [ ] **Step 1: Update log level logic**

```c
// Lines 29-31, change from:
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

---

### Task 7: Update cmd_owner.c log level setting

**Files:**
- Modify: `obmmctl/src/cmd_owner.c:43-45`

- [ ] **Step 1: Update log level logic**

```c
// Lines 43-45, change from:
    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }
// To:
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }
```

---

### Task 8: Final verification and commit

**Files:**
- All modified files

- [ ] **Step 1: Run full test suite**

Run: `cd /home/ubuntu/workspace/github/ub-examples/obmmctl && make clean && make test 2>&1`
Expected: All tests pass

- [ ] **Step 2: Commit changes**

```bash
git add obmmctl/include/cli_parser.h obmmctl/src/cli_parser.c obmmctl/src/cmd_lend.c obmmctl/src/cmd_borrow.c obmmctl/src/cmd_status.c obmmctl/src/cmd_owner.c obmmctl/tests/test_cli_parser.c obmmctl/tests/test_runner.c
git commit -m "$(cat <<'EOF'
feat: support -v/-vv verbose level control

Replace bool verbose with int verbose_count counter:
- -v sets INFO level
- -vv sets DEBUG level

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```