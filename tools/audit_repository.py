#!/usr/bin/env python3
"""Read-only Phase 0 inventory; never claims that the game builds or tests pass."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
from urllib.parse import unquote, urlsplit

REQUIRED_DOCUMENTS = (
    "README.md",
    "AGENTS.md",
    "Docs/00_START_HERE.md",
    "Docs/01_GAME_PASSPORT.md",
    "Docs/02_DESIGN_DECISIONS.md",
    "Docs/03_CURRENT_STATUS.md",
    "Docs/04_OPEN_QUESTIONS.md",
    "Docs/05_ROADMAP.md",
    "Docs/06_CHANGELOG.md",
    "Docs/07_HANDOFF.md",
)
UE_TOOLS = ("UnrealEditor", "UnrealEditor-Cmd", "UnrealBuildTool", "RunUAT.sh")
OTHER_TOOLS = ("dotnet", "msbuild", "cl", "clang++", "g++", "cmake", "ninja", "python3")
LINK_PATTERN = re.compile(r"!?\[[^\]\n]*\]\(\s*(<[^>\n]+>|[^\s)]+)(?:\s+[^)]*)?\)")
FENCE_PATTERN = re.compile(r"^\s*(`{3,}|~{3,})")


def build_inventory(paths: list[str]) -> dict:
    files = sorted(set(paths))

    def ending(*suffixes: str) -> list[str]:
        return [path for path in files if path.lower().endswith(suffixes)]

    return {
        "tracked_file_count": len(files),
        "tracked_files": files,
        "extensions": dict(sorted(Counter(Path(path).suffix.lower() or "[none]" for path in files).items())),
        "unreal_projects": ending(".uproject"),
        "unreal_plugins": ending(".uplugin"),
        "unreal_assets": ending(".uasset", ".umap"),
        "unreal_build_descriptors": ending(".build.cs", ".target.cs"),
        "cpp_files": ending(".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"),
        "csharp_files": ending(".cs"),
        "python_test_candidates": [path for path in ending(".py") if Path(path).name.startswith("test_")],
        "project_directories": {
            name: any(path.startswith(name + "/") for path in files)
            for name in ("Source", "Content", "Config", "Plugins")
        },
    }


def without_fenced_code(text: str):
    fence = None
    for number, line in enumerate(text.splitlines(), 1):
        match = FENCE_PATTERN.match(line)
        if match:
            marker = match.group(1)
            if fence is None:
                fence = marker
            elif marker[0] == fence[0] and len(marker) >= len(fence):
                fence = None
            continue
        if fence is None:
            yield number, line


def check_markdown_links(root: Path, paths: list[str]) -> dict:
    """Check simple inline links, not a full CommonMark/reference-link parser."""
    root = root.resolve()
    checked = 0
    issues = []
    for name in sorted(set(paths)):
        source = root / name
        if source.suffix.lower() != ".md":
            continue
        if not source.is_file():
            issues.append({"file": name, "line": None, "target": None, "status": "MISSING_FILE"})
            continue
        for number, line in without_fenced_code(source.read_text(encoding="utf-8")):
            for match in LINK_PATTERN.finditer(line):
                target = match.group(1).strip("<>")
                parts = urlsplit(target)
                if parts.scheme or parts.netloc or target.startswith("#") or not parts.path:
                    continue
                checked += 1
                destination = (source.parent / unquote(parts.path)).resolve()
                if not destination.is_relative_to(root):
                    status = "OUTSIDE_REPOSITORY"
                elif not destination.exists():
                    status = "MISSING"
                else:
                    continue
                issues.append({"file": name, "line": number, "target": target, "status": status})
    return {"checked_local_links": checked, "issues": issues}


def inspect_pdf(path: Path) -> dict:
    with path.open("rb") as stream:
        header = stream.read(1024)
    match = re.search(rb"%PDF-(\d+\.\d+)", header)
    return {
        "path": path.name,
        "bytes": path.stat().st_size,
        "header_version": match.group(1).decode("ascii") if match else None,
        "content_verified": False,
    }


def git(root: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(root), *arguments],
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def collect_audit(root: Path, which=shutil.which) -> dict:
    root = root.resolve()
    paths = [path for path in git(root, "ls-files", "-z").split("\0") if path]
    inventory = build_inventory(paths)
    missing = [path for path in REQUIRED_DOCUMENTS if path not in paths or not (root / path).is_file()]
    links = check_markdown_links(root, paths)
    tools = {name: which(name) for name in UE_TOOLS + OTHER_TOOLS}
    pdfs = [inspect_pdf(root / path) for path in paths if path.lower().endswith(".pdf") and (root / path).is_file()]
    failed = bool(missing or links["issues"])
    return {
        "schema_version": 1,
        "audit_execution": "FAIL" if failed else "PASS",
        "scope": "Tracked Git inventory, document links and current sandbox only; not implementation certification.",
        "git": {
            "root": str(root),
            "branch": git(root, "branch", "--show-current").strip() or None,
            "head": git(root, "rev-parse", "HEAD").strip(),
            "worktree_dirty": bool(git(root, "status", "--porcelain")),
        },
        "environment": {
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "python": platform.python_version(),
        },
        "inventory": inventory,
        "required_documents": {"status": "FAIL" if missing else "PASS", "missing": missing},
        "markdown_links": {**links, "status": "FAIL" if links["issues"] else "PASS"},
        "pdf_header_checks": pdfs,
        "docx_files": [path for path in paths if path.lower().endswith(".docx")],
        "tools": tools,
        "unreal_tool_found_in_path": any(tools[name] for name in UE_TOOLS),
        "implementation_checks": {
            "unreal_build": "NOT RUN",
            "unreal_editor": "NOT RUN",
            "gameplay_tests": "NOT RUN",
            "save_migration_tests": "NOT RUN",
            "replay_determinism_tests": "NOT RUN",
        },
    }


def print_summary(report: dict) -> None:
    inventory = report["inventory"]
    print(f"Audit execution: {report['audit_execution']} (not a game build/test result)")
    print(f"Branch: {report['git']['branch'] or '(detached)'}; HEAD: {report['git']['head']}")
    print(f"Tracked files: {inventory['tracked_file_count']}; dirty: {report['git']['worktree_dirty']}")
    print(f"UE projects: {len(inventory['unreal_projects'])}; UE assets: {len(inventory['unreal_assets'])}")
    print(f"Required documents: {report['required_documents']['status']}")
    print(f"Local Markdown links: {report['markdown_links']['status']} ({report['markdown_links']['checked_local_links']} checked)")
    print(f"UE tools found in PATH: {report['unreal_tool_found_in_path']}")
    for item in report["required_documents"]["missing"]:
        print(f"MISSING DOCUMENT: {item}")
    for item in report["markdown_links"]["issues"]:
        print(f"LINK {item['status']}: {item['file']}:{item['line']} -> {item['target']}")
    for name, result in report["implementation_checks"].items():
        print(f"{name}: {result}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--json", action="store_true", help="Print the complete machine-readable report.")
    arguments = parser.parse_args()
    try:
        report = collect_audit(arguments.root)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"Audit execution: FAIL\n{error}", file=sys.stderr)
        return 1
    if arguments.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print_summary(report)
    return 1 if report["audit_execution"] == "FAIL" else 0


if __name__ == "__main__":
    raise SystemExit(main())
