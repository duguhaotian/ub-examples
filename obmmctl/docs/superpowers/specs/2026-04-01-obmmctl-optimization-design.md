# obmmctl 优化设计文档

**日期**: 2026-04-01
**版本**: 1.0
**状态**: 待审核

## 1. 概述

### 1.1 背景

obmmctl 是一个基于 OBMM（Ownership-Based Memory Management）库的共享内存管理 CLI 工具。当前版本存在以下问题：

- 使用 stub 实现，未对接真实 libobmm 库
- 参数设计不友好，用户需要手动输入过多信息
- 缺少自动参数获取机制
- 缺少完善的日志和错误处理系统

### 1.2 目标

1. **功能完善**: 对接真实 libobmm 库，简化用户输入，自动从 sysfs 读取硬件参数
2. **质量改进**: 增强错误处理，引入日志系统，改善代码架构

### 1.3 约束

- 保持 C11 标准
- 支持 Linux 平台
- 兼容 openEuler 环境
- stub 仅用于单元测试

## 2. 架构设计

### 2.1 目录结构

```
obmmctl/
├── Makefile              # 构建系统（支持两种模式）
├── README.md             # 使用文档
├── include/
│   ├── obmmctl.h         # 主头文件（版本、错误码、宏）
│   ├── cli_parser.h      # CLI 参数解析
│   ├── sysfs_reader.h    # sysfs 参数读取（新增）
│   ├── libobmm_wrap.h    # libobmm 封装（新增，替代 obmm_wrapper.h）
│   ├── log.h             # 日志系统（新增）
│   └── obmm_stubs.h      # stub API（仅 UT 使用）
├── src/
│   ├── main.c            # 入口、命令分发
│   ├── cli_parser.c      # 参数解析（简化）
│   ├── sysfs_reader.c    # sysfs 读取实现（新增）
│   ├── libobmm_wrap.c    # libobmm 封装实现（新增）
│   ├── log.c             # 日志实现（新增）
│   ├── cmd_lend.c        # lend 命令
│   ├── cmd_borrow.c      # borrow 命令
│   ├── cmd_owner.c       # set-owner 命令
│   ├── cmd_status.c      # status 命令
│   ├── cmd_help.c        # help 命令
├── stubs/
│   └── libobmm_stubs.c   # stub 实现（仅 UT）
├── tests/
│   ├── test_runner.c     # 测试入口
│   ├── test_cli_parser.c # CLI 解析测试
│   ├── test_sysfs_reader.c  # sysfs 读取测试（新增）
│   └── test_libobmm_wrap.c  # libobmm 封装测试（新增）
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-04-01-obmmctl-optimization-design.md  # 本文档
```

### 2.2 模块职责

| 模块 | 职责 | 依赖 |
|------|------|------|
| main.c | 入口、命令分发、全局选项处理 | cli_parser, log |
| cli_parser.c | CLI 参数解析、help 输出 | obmmctl.h |
| sysfs_reader.c | 从 sysfs 读取 eid、primary_cna 等 | log |
| libobmm_wrap.c | 封装 libobmm API，提供简化接口 | sysfs_reader, libobmm, log |
| log.c | 日志输出、级别控制 | 无 |
| cmd_lend.c | lend 命令实现 | cli_parser, sysfs_reader, libobmm_wrap, log |
| cmd_borrow.c | borrow 命令实现 | cli_parser, sysfs_reader, libobmm_wrap, log |
| cmd_owner.c | set-owner 命令实现 | cli_parser, libobmm_wrap, log |
| cmd_status.c | status 命令实现 | cli_parser, libobmm_wrap, log |

### 2.3 数据流

```
用户输入命令 → CLI 解析 → sysfs 读取硬件参数 → libobmm_wrap 调用真实 API → 输出结果
```

## 3. 命令设计

### 3.1 lend 命令

```bash
obmmctl lend --eid <controller_path> [--size <size>] [--flags <flags>]
```

**参数说明**:

| 参数 | 必选 | 默认值 | 说明 |
|------|------|--------|------|
| `--eid` | 是 | - | 控制器完整路径，如 `/sys/devices/ub_bus_controller0/0` |
| `--size` | 否 | 128MB | 内存大小，支持 K/M/G 后缀 |
| `--flags` | 否 | `OBMM_EXPORT_FLAG_ALLOW_MMAP` | 导出 flags |

**执行流程**:
1. 解析 `--eid` 参数，获取控制器路径
2. 从路径读取 `eid` 文件 → 填充 `desc.deid[16]`
3. 使用默认或用户指定的 size/flags
4. 调用 `obmm_export()`，由内核分配内存
5. 输出返回的 `mem_id`、`addr`（物理地址）、`tokenid` 等信息

**输出信息**:
- `mem_id`: 内存 ID，用于后续 unlend 操作
- `addr`: 分配的物理地址，**需要提供给远端用于 borrow 操作**
- `tokenid`: 令牌 ID

### 3.2 borrow 命令

```bash
obmmctl borrow --eid <controller_path> --addr <remote_addr> [--size <size>] [--flags <flags>]
```

**参数说明**:

| 参数 | 必选 | 默认值 | 说明 |
|------|------|--------|------|
| `--eid` | 是 | - | 本端控制器完整路径，如 `/sys/devices/ub_bus_controller0/0` |
| `--addr` | 是 | - | 远端内存物理地址（远端 lend 返回的 addr 值） |
| `--size` | 是 | - | 内存大小（需与远端 lend 的大小一致） |
| `--flags` | 否 | `OBMM_IMPORT_FLAG_ALLOW_MMAP` | 导入 flags |

**执行流程**:
1. 解析 `--eid` 参数，获取本端控制器路径
2. 从路径读取 `eid` 文件 → 填充 `desc.seid[16]`
3. 从同路径读取 `primary_cna` 文件 → 填充 `desc.scna`
4. 从参数获取远端物理地址 → 填充 `desc.addr`
5. 使用用户指定的 size/flags
6. 调用 `obmm_import()`
7. 输出返回的 `mem_id` 和相关信息

**重要说明**:
- `--addr` 是远端 lend 操作返回的物理地址，需要从远端获取
- `--size` 需要与远端 lend 的大小一致
- 典型使用场景：远端先执行 `obmmctl lend` 获取 addr，本端使用该 addr 执行 `obmmctl borrow`

### 3.3 其他命令

保持现有设计不变：

- `unlend --id <mem_id>`: 取消 lend
- `unborrow --id <mem_id>`: 取消 borrow
- `set-owner --id <mem_id> --node <node_id>`: 设置所有权
  - 内部流程：打开 `/dev/obmm_shmdev{mem_id}` 获取 fd，调用 `obmm_set_ownership(fd, start, end, prot)`
- `status [--id <mem_id>]`: 查询状态
- `help [command]`: 显示帮助

### 3.4 全局选项

- `--version`, `-v`: 显示版本
- `--verbose`: 详细输出模式（日志级别 INFO）
- `--help`, `-h`: 显示帮助

## 4. sysfs_reader 模块

### 4.1 接口定义

```c
// sysfs_reader.h

#ifndef SYSFS_READER_H
#define SYSFS_READER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t eid[16];       // 128-bit EID（低4字节从文件读取，高位补零）
    uint32_t primary_cna; // SCNA 值（从 primary_cna 文件读取，格式如 0x401）
    int numa_id;           // NUMA node ID
    int ummu_map;          // UMMU mapping
    bool valid;            // 读取成功标志
} controller_info_t;

// 初始化 controller_info
void controller_info_init(controller_info_t *info);

// 从控制器路径读取所有信息
// controller_path: 如 "/sys/devices/ub_bus_controller0/0"
// 返回: 0 成功，负值失败（errno 已设置）
int read_controller_info(const char *controller_path, controller_info_t *info);

// 单独读取特定属性
int read_eid(const char *path, uint8_t *eid);
int read_primary_cna(const char *path, uint32_t *cna);
int read_numa_id(const char *path, int *numa);
int read_ummu_map(const char *path, int *ummu_map);

#endif /* SYSFS_READER_H */
```

### 4.2 读取逻辑

参考 libobmm 的 vendor_adaptor.c 实现：

**eid 读取**:
- 文件路径: `{controller_path}/eid`
- 文件内容: 整数格式（支持 hex 如 `0x64` 或 decimal 如 `100`）
- 存储: 将整数填入 `eid[0-3]`（小端序），`eid[4-15]` 补零

**primary_cna 读取**:
- 文件路径: `{controller_path}/primary_cna`
- 文件内容: hex 格式如 `0x401`
- 存储: 直接作为 `uint32_t`

### 4.3 错误处理

| 错误条件 | errno | 错误信息 |
|----------|-------|----------|
| 文件不存在 | `ENOENT` | "controller path not found: {path}" |
| 读取失败 | `EIO` | "failed to read from {path}" |
| 解析失败 | `EINVAL` | "invalid value in {path}" |
| 路径格式错误 | `EINVAL` | "invalid controller path format" |

## 5. libobmm_wrap 模块

### 5.1 接口定义

```c
// libobmm_wrap.h

#ifndef LIBOBMM_WRAP_H
#define LIBOBMM_WRAP_H

#include <stdint.h>
#include <stdbool.h>
#include <libobmm.h>
#include "sysfs_reader.h"

typedef struct {
    mem_id id;                      // 内存 ID
    struct obmm_mem_desc desc;      // 内存描述符
    bool is_export;                 // true: lend/export, false: borrow/import
} obmm_handle_t;

// 初始化 handle
void obmm_handle_init(obmm_handle_t *handle);

// lend 操作 - export 内存
// ctrl: 控制器信息（deid 从 ctrl->eid 获取）
// size: 内存大小（必须 2MB 对齐）
// flags: 默认 OBMM_EXPORT_FLAG_ALLOW_MMAP
// 返回: 0 成功，负值失败
int obmm_do_lend(const controller_info_t *ctrl, size_t size,
                 unsigned long flags, obmm_handle_t *handle);

// borrow 操作 - import 内存
// ctrl: 控制器信息（seid 从 ctrl->eid 获取，scna 从 ctrl->primary_cna 获取）
// remote_addr: 远端内存物理地址（远端 lend 返回的值）
// size: 内存大小
// flags: 默认 OBMM_IMPORT_FLAG_ALLOW_MMAP
// 返回: 0 成功，负值失败
int obmm_do_borrow(const controller_info_t *ctrl, uint64_t remote_addr, size_t size,
                   unsigned long flags, obmm_handle_t *handle);

// unlend 操作
int obmm_do_unlend(mem_id id, unsigned long flags);

// unborrow 操作
int obmm_do_unborrow(mem_id id, unsigned long flags);

// 查询状态
int obmm_query_status(mem_id id, struct obmm_mem_desc *desc);

// 清理 handle（用于 RAII 或手动清理）
void obmm_handle_cleanup(obmm_handle_t *handle);

#endif /* LIBOBMM_WRAP_H */
```

### 5.2 API 映射

| obmmctl 操作 | libobmm API | 参数来源 |
|--------------|-------------|----------|
| lend | `obmm_export()` | `ctrl->eid` → `desc.deid`，内核分配内存返回 `desc.addr` |
| borrow | `obmm_import()` | `ctrl->eid` → `desc.seid`，`ctrl->primary_cna` → `desc.scna`，用户输入 `addr`、`size` |
| unlend | `obmm_unexport()` | mem_id |
| unborrow | `obmm_unimport()` | mem_id |
| set-owner | `obmm_set_ownership()` | fd（从 `/dev/obmm_shmdev{mem_id}` 获取）, start, end, prot |
| status | `obmm_query_memid_by_pa()` 或直接查询 | mem_id |

**API 选择说明**:
- **lend**: 使用 `obmm_export()`，由内核自动分配内存，适用于大多数场景
- **borrow**: 使用 `obmm_import()`，需要提供远端物理地址

### 5.3 内存对齐

所有操作要求 2MB 对齐：
- size 必须是 2MB 的倍数
- 地址必须是 2MB 对齐
- 不满足时返回错误 `EINVAL`

## 6. 日志系统

### 6.1 接口定义

```c
// log.h

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

typedef enum {
    LOG_LEVEL_ERROR = 0,   // 错误，始终输出
    LOG_LEVEL_WARN  = 1,   // 警告
    LOG_LEVEL_INFO  = 2,   // 信息（verbose 模式）
    LOG_LEVEL_DEBUG = 3,   // 调试（debug 模式）
} log_level_t;

// 设置/获取日志级别
void log_set_level(log_level_t level);
log_level_t log_get_level(void);

// 核心输出函数
void log_output(log_level_t level, const char *fmt, ...);

// 日志宏
#define LOG_ERROR(fmt, ...)  log_output(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   log_output(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   log_output(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)  log_output(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

// EID 格式化宏（128-bit 完整显示）
#define EID_FMT16 "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define EID_ARGS16(e) (e)[0], (e)[1], (e)[2], (e)[3], (e)[4], (e)[5], (e)[6], (e)[7], \
                      (e)[8], (e)[9], (e)[10], (e)[11], (e)[12], (e)[13], (e)[14], (e)[15]

// EID 格式化宏（64-bit 简化显示，参考 libobmm）
#define EID_FMT64 "%02x%02x%02x%02x%02x%02x%02x%02x"
#define EID_ARGS64(e) (e)[0], (e)[1], (e)[2], (e)[3], (e)[4], (e)[5], (e)[6], (e)[7]

#endif /* LOG_H */
```

### 6.2 输出格式

```
[ERROR] obmmctl: failed to read eid from /sys/devices/ub_bus_controller0/0/eid: No such file or directory
[WARN]  obmmctl: size 64MB is not 2MB aligned, will be rounded to 64MB
[INFO]  obmmctl: lend success, mem_id=1001, size=128MB
[DEBUG] obmmctl: controller path=/sys/devices/ub_bus_controller0/0, eid=0102030405060708...
```

### 6.3 级别控制

| 控制方式 | 日志级别 |
|----------|----------|
| 默认 | `LOG_LEVEL_ERROR` |
| `--verbose` 或 `-v` | `LOG_LEVEL_INFO` |
| 环境变量 `OBMMCTL_DEBUG=1` | `LOG_LEVEL_DEBUG` |

## 7. 构建系统

### 7.1 Makefile 设计

```makefile
# Makefile

CC = gcc
CFLAGS_COMMON = -Wall -Wextra -std=c11 -I./include
CFLAGS_PROD = $(CFLAGS_COMMON) -DUSE_REAL_LIBOBMM
CFLAGS_TEST = $(CFLAGS_COMMON)

# 目标
TARGET = obmmctl
TEST_TARGET = obmmctl_test

# 生产版本：链接真实 libobmm
all: $(TARGET)

$(TARGET): $(PROD_OBJS)
    $(CC) $(CFLAGS_PROD) -o $@ $^ -lobmm

# 测试版本：使用 stubs
test: $(TEST_TARGET)
    ./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(STUB_OBJS) $(filter-out main.o,$(PROD_OBJS))
    $(CC) $(CFLAGS_TEST) -o $@ $^

clean:
    rm -rf build $(TARGET) $(TEST_TARGET)
```

### 7.2 构建模式

| 命令 | 说明 | 依赖 |
|------|------|------|
| `make` | 生产版本 | 需要 `libobmm` 和 `<ub/obmm.h>` |
| `make test` | 单元测试 | 使用 stubs，无外部依赖 |
| `make clean` | 清理 | 无 |

### 7.3 条件编译

通过 `-DUSE_REAL_LIBOBMM` 区分：

```c
#ifdef USE_REAL_LIBOBMM
#include <libobmm.h>
#else
#include "obmm_stubs.h"
#endif
```

## 8. 测试策略

### 8.1 单元测试

使用 stub 进行以下测试：

| 测试模块 | 测试内容 |
|----------|----------|
| test_cli_parser | 参数解析、默认值、错误处理 |
| test_sysfs_reader | 路径解析、文件读取、错误模拟 |
| test_libobmm_wrap | API 调用逻辑（使用 stub） |

### 8.2 集成测试

需要真实硬件环境：
- lend/borrow 完整流程
- 真实 sysfs 读取
- libobmm API 调用

### 8.3 测试覆盖率

目标：核心模块 > 80%

## 9. 错误处理策略

### 9.1 错误码

保持现有错误码设计：

| 代码 | 名称 | 说明 |
|------|------|------|
| 0 | `EXIT_SUCCESS_CODE` | 成功 |
| 1 | `EXIT_FAILURE_CODE` | 一般错误 |
| 2 | `EXIT_USAGE_ERROR` | 参数错误 |
| 3 | `EXIT_OBMM_ERROR` | OBMM API 错误 |
| 126 | `EXIT_PERMISSION_DENIED` | 权限不足 |
| 127 | `EXIT_COMMAND_NOT_FOUND` | 命令不存在 |

### 9.2 错误信息

- 使用 LOG_ERROR 输出详细错误信息
- 包含 errno 解释（使用 strerror）
- 包含失败的操作和路径信息

示例：
```
[ERROR] obmmctl: lend failed: obmm_export returned OBMM_INVALID_MEMID
[ERROR] obmmctl: read_controller_info failed: cannot open /sys/devices/ub_bus_controller0/0/eid: Permission denied
```

## 10. 迁移计划

### 10.1 文件变更

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `sysfs_reader.c/h` | sysfs 读取模块 |
| 新增 | `libobmm_wrap.c/h` | 替代 obmm_wrapper |
| 新增 | `log.c/h` | 日志系统 |
| 修改 | `cli_parser.c/h` | 参数简化、默认值 |
| 修改 | `cmd_lend.c` | 使用新接口 |
| 修改 | `cmd_borrow.c` | 使用新接口 |
| 修改 | `Makefile` | 双构建模式 |
| 删除 | `obmm_wrapper.c/h` | 被 libobmm_wrap 替代 |
| 保留 | `obmm_stubs.h` | 仅 UT 使用 |

### 10.2 兼容性

- 命令名称保持不变
- 其他命令参数保持不变
- 输出格式保持兼容

## 11. 依赖

### 11.1 外部依赖

| 依赖 | 版本要求 | 说明 |
|------|----------|------|
| libobmm | 参考 atomgit.com/openeuler/obmm | 核心库 |
| ub/obmm.h | 内核头文件 | OBMM ioctl 定义 |
| gcc | C11 支持 | 编译器 |

### 11.2 系统要求

- Linux 内核支持 OBMM
- `/sys/devices/ub_bus_controller*` sysfs 接口
- `/dev/obmm` 设备文件