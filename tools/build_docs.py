#!/usr/bin/env python3
"""Build Gino documentation with the pinned sdk_build_doc_template checkout."""

import argparse
from html.parser import HTMLParser
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from urllib.parse import unquote, urlsplit

import yaml


ROOT = Path(__file__).resolve().parents[1]
TEMPLATE_REVISION = "6c5b5b040b45dbb936bf389d90ecbaadc32b42de"
IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".gif", ".svg", ".webp"}


def documentation_files():
    files = set(ROOT.glob("README*.md"))
    files.update((ROOT / "docs").glob("*.md"))
    files.update((ROOT / "docs").glob("*.pdf"))
    files.update((ROOT / "tools" / "mdk_pack").glob("*.pack"))
    asset_dirs = [ROOT / "figures"]
    projects = sorted((ROOT / "projects").glob("*/SConstruct"))
    if not projects:
        raise ValueError("No example projects found")
    for entry in projects:
        project = entry.parent
        for name in ("README.md", "README_zh.md"):
            readme = project / name
            if not readme.is_file():
                raise FileNotFoundError(readme)
            files.add(readme)
        asset_dirs.extend((project / "figures", project / "image"))
    for directory in asset_dirs:
        if directory.is_dir():
            files.update(
                path for path in directory.rglob("*")
                if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES
            )
    return sorted(files)


class LocalReferences(HTMLParser):
    def __init__(self):
        super().__init__()
        self.targets = set()

    def handle_starttag(self, tag, attrs):
        for name, value in attrs:
            if value and (name == "src" or (name == "href" and tag in {"a", "link"})):
                self.targets.add(value)


def check_site(output):
    missing = []
    for required in ("index.html", "README.html", "README_zh.html"):
        if not (output / required).is_file():
            missing.append(required)
    pages = sorted(output.rglob("*.html"))
    for page in pages:
        parser = LocalReferences()
        parser.feed(page.read_text(encoding="utf-8"))
        for target in sorted(parser.targets):
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            path = unquote(parsed.path)
            resolved = (output / path.lstrip("/") if path.startswith("/")
                        else page.parent / path).resolve()
            if resolved.is_dir():
                resolved /= "index.html"
            if not resolved.is_relative_to(output) or not resolved.is_file():
                missing.append(f"{page.relative_to(output)}: {target}")
    if missing:
        raise ValueError("Missing local documentation targets:\n" + "\n".join(missing))
    print(f"Checked {len(pages)} HTML pages: all local links and assets exist")


def build(template, output):
    revision = subprocess.check_output(
        ["git", "-C", str(template), "rev-parse", "HEAD"], text=True
    ).strip()
    if revision != TEMPLATE_REVISION:
        raise ValueError(f"Documentation template must be at {TEMPLATE_REVISION}")
    # Only replace the generated site; never clean an arbitrary caller path.
    expected_output = (ROOT / "docs" / "_build" / "html").resolve()
    if output != expected_output:
        raise ValueError(f"Output must be {expected_output}")
    with tempfile.TemporaryDirectory(prefix="gino-docs-") as temporary:
        workspace = Path(temporary)
        source = workspace / "source"
        source.mkdir()
        template_source = template / "source"
        for path in template_source.glob("*.py"):
            shutil.copy2(path, source / path.name)
        for name in ("utils", "_static", "_templates"):
            shutil.copytree(template_source / name, source / name,
                            ignore=shutil.ignore_patterns("__pycache__"))
        shutil.copy2(template_source / "requirements.txt", source / "requirements.txt")
        shutil.copy2(ROOT / "docs" / "config" / "site_config.yaml", source / "config.yaml")
        with (source / "conf.py").open("a", encoding="utf-8") as config:
            config.write("\n" + (ROOT / "docs" / "sphinx_overrides.py").read_text(encoding="utf-8"))
        files = documentation_files()
        for path in files:
            destination = workspace / "content" / path.relative_to(ROOT)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, destination)
        # Explicit directory homes avoid the template's reserved index.html
        # rewrite when merging English pages into a Chinese-default site.
        site_config = yaml.safe_load((source / "config.yaml").read_text(encoding="utf-8"))
        for directory, category in site_config["categories"].items():
            for filename, title_key in (("README.md", "name_en"), ("README_zh.md", "name")):
                homepage = workspace / "content" / directory / filename
                if not homepage.exists():
                    homepage.parent.mkdir(parents=True, exist_ok=True)
                    homepage.write_text(f"# {category[title_key]}\n", encoding="utf-8")
        environment = dict(os.environ, PYTHONUTF8="1")
        subprocess.run(
            [sys.executable, str(source / "build_local.py"), "--no-pdf", "--no-auto-install"],
            cwd=source, env=environment, check=True,
        )
        site = source / "_build" / "html"
        # Preserve relative download URLs and unreferenced guide attachments.
        for path in files:
            if path.suffix.lower() not in {".md", ".rst"}:
                destination = site / path.relative_to(ROOT)
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, destination)
        (site / ".nojekyll").touch()
        check_site(site.resolve())
        if output.exists():
            shutil.rmtree(output)
        shutil.copytree(site, output)
    print(f"Documentation: {output / 'index.html'}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--template", type=Path, required=True,
                        help="Path to sdk_build_doc_template at the pinned revision")
    args = parser.parse_args()
    build(args.template.resolve(), (ROOT / "docs" / "_build" / "html").resolve())


if __name__ == "__main__":
    main()
