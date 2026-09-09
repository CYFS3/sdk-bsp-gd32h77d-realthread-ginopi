# 文档构建与本地预览

本仓库完整引入 [sdk_build_doc_template](https://github.com/kurisaW/sdk_build_doc_template)
的 `source/` 目录，版本为 `6c5b5b040b45dbb936bf389d90ecbaadc32b42de`。
构建脚本、`utils/`、`_static/`、`_templates/` 和模板测试均保留原文件。
适配记录见 `source/UPSTREAM.json`。

## 本地预览

推荐 Python 3.11。只预览网页时使用模板原生的 `--no-pdf`，无需安装 LaTeX：

```sh
python -m pip install -r source/requirements.txt
cd source
python build_local.py --clean --no-pdf --serve --port 8765
```

访问 <http://localhost:8765>。网页目录、搜索、图片和中英文切换均由模板提供。
按 `Ctrl+C` 停止服务。只生成 HTML、不启动服务时去掉 `--serve`。
产物位于 `source/_build/html/`。

需要同时生成中英文 PDF 时，先安装 XeLaTeX 和 `source/config.yaml` 中指定的字体，
然后去掉 `--no-pdf`：

```sh
cd source
python utils/pdf_environment.py --no-auto-install
python build_local.py --clean --serve --port 8765
```

## 配置与文档

- `source/config.yaml`：Gino 产品信息、文档分类、语言与 PDF 字体。
- `source/conf.py`：保留模板配置，仅追加标题锚点配置，并关闭不适配本仓库路径的 GitHub 编辑按钮。
- `docs/project-guide/README*.md`：SDK 使用指南，包含目录、工程选择、硬件连接和编译下载步骤。
- `projects/*/README*.md`：示例工程文档，保留原位置。
- `.github/versions.json`：版本分支、显示名称、发布路径和默认版本。

使用模板的 `project_catalog` 内容发现与 `categories` 导航配置，只收录明确配置的
1 组 SDK 使用指南和 16 组示例入口及其图片。不会扫描第三方库的 README，也不会编译 BSP。
根 README 用于 GitHub 仓库介绍；在线首页由模板根据产品信息和分类自动生成。

## 多版本发布

在对应分支上使用模板原生入口：

```sh
cd source
python build.py --validate
python build.py --clean
```

模板通过 Git worktree 构建 `.github/versions.json` 声明的分支，产物位于
`source/source_build/html/`。根入口跳转到 `latest/index.html`。
增加历史版本前，确保相应分支也包含 `source/` 和文档配置。

CI 在独立 runner 上将当前提交作为 `latest` 对应分支进行构建，保证 PR 检查读取的是
PR 文档。`tools/check_docs.py` 只校验生成结果，不参与模板构建。

GitHub Pages 设置为 **Deploy from a branch / gh-pages / (root)**。
工作流使用模板相同的 `peaceiris/actions-gh-pages@v4`，把完整产物发布到 `gh-pages`。
只有默认分支会执行发布；修复分支的手动运行只上传 `docs-build` 预览产物。
