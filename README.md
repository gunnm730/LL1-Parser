# LL1-Parser

## LL(1) 语法分析器 / LL(1) Syntax Analyzer

一个用 C++ 实现的 LL(1) 语法分析器，用于编译原理实验二。

An LL(1) syntax analyzer implemented in C++ for Compiler Theory Lab 2.

---

## 功能特性 / Features

| 功能 / Feature | 描述 / Description |
|---|---|
| 文法读取 / Grammar Input | 从文件读取文法规则 / Read grammar rules from file |
| 左递归消除 / Left Recursion Elimination | 自动检测并消除直接和间接左递归 / Automatically detect and eliminate direct and indirect left recursion |
| 回溯消除 / Backtracking Elimination | 消除文法中的回溯现象 / Eliminate backtracking in grammar |
| 无用产生式消除 / Useless Production Elimination | 消除无法到达或无法终止的产生式 / Remove unreachable or non-terminating productions |
| FIRST 集计算 / FIRST Set Computation | 计算所有非终结符的 FIRST 集 / Compute FIRST sets for all non-terminals |
| FOLLOW 集计算 / FOLLOW Set Computation | 计算所有非终结符的 FOLLOW 集 / Compute FOLLOW sets for all non-terminals |
| LL(1) 分析表构建 / LL(1) Parsing Table | 构建预测分析表 / Build predictive parsing table |
| 语法分析 / Syntax Analysis | 对输入字符串进行语法分析 / Perform syntax analysis on input strings |
| 推导过程展示 / Derivation Steps | 显示完整的推导过程 / Display complete derivation process |

---

## 项目结构 / Project Structure

```
LL1-Parser/
├── src/
│   ├── Main.cpp                  # 主程序入口 / Main entry point
│   ├── Syntactic_Analysis.h     # 头文件 / Header file
│   └── Syntactic_Analysis.cpp   # 实现文件 / Implementation
└── README.md
```

---

## 使用方法 / Usage

### 1. 准备文法文件 / Prepare Grammar File

创建一个文本文件，包含文法产生式，格式如下：

Create a text file with grammar productions in the following format:

```
S::=aAb
A::=c|@
```

### 2. 文法格式说明 / Grammar Format

| 符号 / Symbol | 含义 / Meaning |
|---|---|
| `::=` | 产生式符号 / Production symbol |
| `\|` | 多个候选项的分隔符 / Separator for multiple alternatives |
| `@` | 空串/epsilon / Empty string / epsilon |

### 3. 编译项目 / Compile

```bash
g++ -std=c++11 src/*.cpp -o LL1-Parser
```

### 4. 运行程序 / Run

```bash
./LL1-Parser
```

### 5. 输入 / Input

程序会提示输入：

The program will prompt for:

1. **文法文件路径** - 输入步骤1准备的文法文件完整路径
   - **Grammar file path** - Enter the full path to the grammar file

2. **待分析字符串** - 输入需要语法分析的字符串
   - **String to analyze** - Enter the string to be analyzed

---

## 示例 / Example

### 文法文件 grammar.txt

```
S::=aAb
A::=c|@
```

### 运行结果 / Output

```
请输入文法文件路径: grammar.txt
请输入待分析字符串: acb

========== 文法处理 ==========
原始文法 / Original Grammar:
S::=aAb
A::=c|@

消除左递归后 / After Left Recursion Elimination:
S::=aAb
A::=c|@

消除回溯后 / After Backtracking Elimination:
S::=aAb
A::=c|@

========== FIRST 集 / FIRST Sets ==========
FIRST(S) = {a}
FIRST(A) = {c,@}

========== FOLLOW 集 / FOLLOW Sets ==========
FOLLOW(S) = {#}
FOLLOW(A) = {b}

========== LL(1) 分析表 / LL(1) Parsing Table ==========
     a    b    c    #
S  aAb
A       Ab   c

========== 推导过程 / Derivation Steps ==========
步骤: S -> aAb
步骤: A -> c
步骤: S -> acb

分析成功! / Analysis Successful!
```

---

## 核心算法 / Core Algorithms

### 1. 左递归消除 / Left Recursion Elimination

- **直接左递归**: `A -> Aα | β` → `A -> βA' , A' -> αA' | @`
- **Direct left recursion**: `A -> Aα | β` → `A -> βA' , A' -> αA' | @`

- **间接左递归**: 通过拓扑排序确定消除顺序
- **Indirect left recursion**: Use topological sort to determine elimination order

### 2. 回溯消除 / Backtracking Elimination

- 提取左公因子，将 `A -> αβ1 | αβ2` 转换为 `A -> αA'`
- Extract left factoring: transform `A -> αβ1 | αβ2` to `A -> αA'`

### 3. FIRST 集计算 / FIRST Set Computation

- 递归计算每个非终结符的 FIRST 集
- Recursively compute FIRST sets for each non-terminal

### 4. FOLLOW 集计算 / FOLLOW Set Computation

- 基于产生式规则计算 FOLLOW 集
- Compute FOLLOW sets based on production rules

### 5. LL(1) 分析表构建 / LL(1) Parsing Table Construction

- 根据 FIRST 和 FOLLOW 集构建预测分析表
- Build predictive parsing table from FIRST and FOLLOW sets

---

## 技术栈 / Tech Stack

- **语言 / Language**: C++11
- **数据结构 / Data Structures**: `std::map`, `std::set`, `std::vector`, `std::string`
- **算法 / Algorithms**: 拓扑排序 (Topological Sort), 递归下降分析 (Recursive Descent Parsing)

---

## 代码结构 / Code Structure

| 文件 / File | 功能 / Function |
|---|---|
| `Main.cpp` | 程序入口，交互界面 / Program entry, interactive interface |
| `Syntactic_Analysis.h` | 类声明和接口定义 / Class declaration and interface definition |
| `Syntactic_Analysis.cpp` | 完整实现 / Complete implementation |

---

## 关键类说明 / Key Class Description

### Syntactic_Analysis 类

负责所有语法分析相关的功能，包括：

- 文法预处理（消除左递归、回溯、无用产生式）
- FIRST/FOLLOW 集计算
- LL(1) 分析表构建
- 语法分析执行

Responsible for all syntax analysis related functions:

- Grammar preprocessing (eliminate left recursion, backtracking, useless productions)
- FIRST/FOLLOW set computation
- LL(1) parsing table construction
- Syntax analysis execution

---

## 实验要求 / Lab Requirements

本项目满足编译原理实验二的要求：

- [x] 读取并解析文法文件
- [x] 消除左递归
- [x] 消除回溯
- [x] 消除无用产生式
- [x] 计算 FIRST 集
- [x] 计算 FOLLOW 集
- [x] 构建 LL(1) 分析表
- [x] 执行语法分析
- [x] 输出推导过程

This project meets the requirements of Compiler Theory Lab 2:

- [x] Read and parse grammar file
- [x] Eliminate left recursion
- [x] Eliminate backtracking
- [x] Eliminate useless productions
- [x] Compute FIRST sets
- [x] Compute FOLLOW sets
- [x] Build LL(1) parsing table
- [x] Perform syntax analysis
- [x] Output derivation steps

---

## 许可证 / License

MIT License
