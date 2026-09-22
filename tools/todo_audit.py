#!/usr/bin/env python3
"""Report reconstruction TODOs across every authored source file.

The scan deliberately includes all supported source languages line-by-line while
excluding vendored WTL headers and Git submodules. It does not infer that every
successful HRESULT is unfinished: TODOs are added only after manual review.
"""
from __future__ import annotations

import argparse
import subprocess
from pathlib import Path
from typing import Iterator, Tuple

ROOT = Path(__file__).resolve().parent.parent
SOURCE_SUFFIXES = {
    '.bat', '.c', '.cc', '.cmake', '.cpp', '.cs', '.cxx', '.def', '.h', '.hpp',
    '.ps1', '.py', '.rc',
}
EXCLUDED_PREFIXES = ('apps/', 'src/WTL/', 'tests/mmr-cli/', 'tests/mmr-gui/', 'tests/mmr-python/')
TODO_PREFIX = 'TODO' + '(reconstruction):'


def authored_source_files() -> Iterator[Path]:
    """Yield tracked, authored source files in a stable order."""
    result = subprocess.run(
        ['git', 'ls-files'], cwd=str(ROOT), check=True, text=True,
        stdout=subprocess.PIPE,
    )
    for relative in sorted(result.stdout.splitlines()):
        if relative.startswith(EXCLUDED_PREFIXES):
            continue
        path = ROOT / relative
        if path.name == 'CMakeLists.txt' or path.suffix.lower() in SOURCE_SUFFIXES:
            yield path


def reconstruction_todos() -> Iterator[Tuple[Path, int, str]]:
    for path in authored_source_files():
        try:
            lines = path.read_text(encoding='utf-8').splitlines()
        except UnicodeDecodeError:
            continue
        for number, line in enumerate(lines, 1):
            if TODO_PREFIX in line:
                yield path.relative_to(ROOT), number, line.strip()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='fail when no reconstruction TODOs are found')
    args = parser.parse_args()
    todos = list(reconstruction_todos())
    for path, line, text in todos:
        print(f'{path}:{line}: {text}')
    print(f'Reviewed source files: {sum(1 for _ in authored_source_files())}')
    print(f'Reconstruction TODOs: {len(todos)}')
    return 1 if args.check and not todos else 0


if __name__ == '__main__':
    raise SystemExit(main())
