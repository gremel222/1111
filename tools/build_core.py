#!/usr/bin/env python3
"""Build and run the engine-independent SimulationCore tests with g++/clang++ or MSVC cl."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", help="Compiler executable; defaults to CXX, cl, g++, or clang++.")
    parser.add_argument("--sanitize", action="store_true", help="Enable address/undefined sanitizers (g++/clang++ only).")
    parser.add_argument("--demo", action="store_true", help="Build and run the console battle demo instead of tests.")
    args = parser.parse_args()
    compiler = args.compiler or os.environ.get("CXX") or next((x for x in ("cl", "g++", "clang++") if shutil.which(x)), None)
    if not compiler or not shutil.which(compiler):
        print("FAIL: no C++ compiler found. Run from a VS Developer PowerShell or install g++.", file=sys.stderr)
        return 2
    sources = ["Source/CodexOfRules/Private/SimulationCore/Battle.cpp",
               "Tests/SimulationCoreDemo.cpp" if args.demo else "Tests/SimulationCoreTests.cpp"]
    stem = "SimulationCoreDemo" if args.demo else "SimulationCoreTests"
    output = ROOT / ".build" / "core"
    output.mkdir(parents=True, exist_ok=True)
    executable = output / (stem + (".exe" if os.name == "nt" else ""))
    name = Path(compiler).stem.lower()
    msvc = name == "cl" or name == "cl.exe"
    command = [compiler]
    if msvc:
        command += ["/nologo", "/std:c++20", "/EHsc", "/W4", "/WX", "/utf-8", "/I", str(ROOT / "Source/CodexOfRules/Public")]
        command += [str(ROOT / path) for path in sources]
        command += ["/Fe:" + str(executable), "/Fo:" + str(output) + os.sep]
    else:
        command += ["-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-g"]
        if args.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        command += ["-I", str(ROOT / "Source/CodexOfRules/Public"), *sources, "-o", str(executable)]
    print("BUILD:", subprocess.list2cmdline(command), flush=True)
    built = subprocess.run(command, cwd=ROOT)
    if built.returncode:
        return built.returncode
    print("RUN:", executable, flush=True)
    return subprocess.run([str(executable)], cwd=ROOT).returncode


if __name__ == "__main__":
    raise SystemExit(main())
