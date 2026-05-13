# 文件结构说明

## 1. 根目录

```text
./
├── README.md
├── CMakeLists.txt
├── docs/
├── sql/
├── src/
├── tests/
├── tools/
├── scripts/
└── generated_data/
```

- `README.md`：项目首页说明。
- `CMakeLists.txt`：顶层 CMake 构建配置。
- `docs/`：设计、运行、阶段结果和报告辅助文档。
- `sql/`：PostgreSQL 建表、索引、触发器、查询、导入导出和性能测试脚本。
- `src/`：Qt/C++ 主程序源码。
- `tests/`：自动验收程序。
- `tools/`：数据生成工具。
- `scripts/`：WSL 运行和输入法辅助脚本。
- `generated_data/`：生成 CSV 和导出文件，本地生成且不提交。

## 2. docs 目录

```text
docs/system_design.md
docs/system_framework.md
docs/usage.md
docs/data_and_tests.md
docs/report_materials.md
docs/planning_and_work.md
docs/project_structure.md
docs/github_upload.md
docs/acceptance_checklist.md
```

## 3. src 目录

```text
src/
├── main.cpp
├── db/
├── model/
├── service/
└── ui/
```

- `db/`：`DatabaseManager` 与 DAO 数据访问层。
- `model/`：`User`、`Genealogy`、`Member` 等数据模型。
- `service/`：认证、统计、族谱树和亲缘链路业务逻辑。
- `ui/`：Qt Widgets 界面。

## 4. tests 目录

```text
tests/stage2_smoke.cpp
```

`Stage2Smoke` 复用主程序连接、DAO 和 Service，验证数据库连接、登录、族谱加载、Dashboard 和成员列表。

## 5. scripts 目录

```text
scripts/run_wsl.sh
scripts/setup_fcitx_wsl.sh
```

- `run_wsl.sh`：从项目根目录启动 `build/GenealogySystem`。
- `setup_fcitx_wsl.sh`：配置 WSLg 下的 fcitx5 拼音输入法。

## 6. 不应上传的目录

```text
build/
generated_data/
```

这些是本地构建产物或生成数据，已经由 `.gitignore` 忽略。
