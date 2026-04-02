# obmmctl 日志级别参数设计

## 背景

当前 obmmctl 仅支持 `-v/--verbose` 开关，设置为 INFO 级别。无法启用 DEBUG 级别日志，调试困难。

## 目标

支持分级 verbose 参数：
- `-v` → INFO 级别
- `-vv` → DEBUG 级别

## 设计

### 1. 数据结构变更

**cli_parser.h:**

将 `bool verbose` 改为 `int verbose_count`：

```c
typedef struct {
    char *controller_path;
    uint64_t remote_addr;
    size_t size;
    uint32_t flags;
    uint64_t mem_id;
    int node_id;
    bool show_help;
    int verbose_count;    /* Verbose level: 0=ERROR, 1=INFO, 2+=DEBUG */
} cmd_options_t;
```

### 2. 解析逻辑变更

**cli_parser.c:**

- `cmd_options_init()`: `verbose_count = 0`
- 解析 `-v` 时: `verbose_count++`
- getopt 格式字符串保持不变（`-v` 不带参数）

```c
case 'v':
    opts->verbose_count++;
    break;
```

### 3. 日志级别设置

各命令（lend、borrow、status 等）执行时根据 `verbose_count` 设置日志级别：

```c
if (opts.verbose_count >= 2) {
    log_set_level(LOG_LEVEL_DEBUG);
} else if (opts.verbose_count >= 1) {
    log_set_level(LOG_LEVEL_INFO);
}
```

### 4. 帮助文档更新

**print_general_usage():**

```
  -v, --verbose   Verbose output (repeat for more: -v=INFO, -vv=DEBUG)
```

### 5. 测试更新

**test_cli_parser.c:**

- 更新初始化测试：`verbose_count == 0`
- 添加 `-v` 计数测试
- 添加 `-vv` 测试

## 文件变更

| 文件 | 变更 |
|------|------|
| `include/cli_parser.h` | `bool verbose` → `int verbose_count` |
| `src/cli_parser.c` | 初始化、解析逻辑、帮助文本 |
| `src/cmd_lend.c` | 日志级别设置逻辑 |
| `src/cmd_borrow.c` | 日志级别设置逻辑 |
| `src/cmd_status.c` | 日志级别设置逻辑 |
| `src/cmd_owner.c` | 日志级别设置逻辑 |
| `tests/test_cli_parser.c` | 更新测试 |

## 使用示例

```bash
# 默认：仅显示错误
obmmctl lend --eid /sys/devices/ub_bus_controller0/0

# 显示信息日志
obmmctl lend -v --eid /sys/devices/ub_bus_controller0/0

# 显示调试日志
obmmctl lend -vv --eid /sys/devices/ub_bus_controller0/0
```