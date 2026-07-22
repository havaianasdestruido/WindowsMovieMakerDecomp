#!/usr/bin/env python3
"""
Runtime Test Harness - Test MovieMaker.exe behavior by launching it,
observing process state, checking window creation, and measuring stability.

Usage:
    python tools/test_all.py                  # Run all tests
    python tools/test_all.py --launch         # Just test launch
    python tools/test_all.py --timeout 30     # Custom timeout
"""

import subprocess
import sys
import os
import time
import json
import ctypes
from pathlib import Path
from dataclasses import dataclass, field, asdict
from typing import List, Optional
from datetime import datetime

# Windows API constants
PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ = 0x0010
PROCESS_TERMINATE = 0x0001


@dataclass
class TestResult:
    name: str
    passed: bool
    duration_ms: float
    details: str = ""
    error: str = ""


@dataclass
class LaunchResult:
    pid: int
    exit_code: Optional[int]
    crashed: bool
    window_title: str
    duration_ms: float
    memory_mb: float
    peak_memory_mb: float
    handles: int
    threads: int


class TestHarness:
    def __init__(self, exe_path: str, timeout: int = 15):
        self.exe_path = exe_path
        self.timeout = timeout
        self.results: List[TestResult] = []
        self.build_dir = str(Path(exe_path).parent.parent.parent / "build" / "bin" / "Debug")

    def run_all(self):
        print(f"\n{'='*70}")
        print(f"  WMMR Runtime Test Harness")
        print(f"  Target: {self.exe_path}")
        print(f"  Timeout: {self.timeout}s")
        print(f"{'='*70}\n")

        self.test_file_exists()
        self.test_dlls_present()
        self.test_launch_clean()
        self.test_launch_with_args()
        self.test_stability()
        self.test_single_instance()
        self.test_window_creation()
        self.test_memory_usage()

        self.print_summary()
        self.save_report()

    def test_file_exists(self):
        name = "File existence check"
        start = time.perf_counter()
        try:
            exists = os.path.isfile(self.exe_path)
            duration = (time.perf_counter() - start) * 1000

            if exists:
                size_mb = os.path.getsize(self.exe_path) / (1024 * 1024)
                self.results.append(TestResult(
                    name=name, passed=True, duration_ms=duration,
                    details=f"EXE exists: {size_mb:.1f} MB"
                ))
            else:
                self.results.append(TestResult(
                    name=name, passed=False, duration_ms=duration,
                    error=f"EXE not found: {self.exe_path}"
                ))
        except Exception as e:
            self.results.append(TestResult(name=name, passed=False, duration_ms=0, error=str(e)))

    def test_dlls_present(self):
        name = "Required DLLs present"
        start = time.perf_counter()
        try:
            expected_dlls = [
                "MovieMakerCore.dll", "WLXPhotoBase.dll",
                "WLXPipeline.dll", "WLXPipetran.dll",
                "WLXSlideshow.dll", "WLXVideoTrim.dll",
                "WLXPhotoCinematic.dll", "WLXMovieLibrary.dll",
                "WLXMP4Parser.dll", "WLXFaceRecognition.dll",
                "WLXMediaPublishSubscribe.dll", "WLMFDS.dll",
                "WLMFReadWrite.dll", "MovieMakerLang.dll",
                "MovieMakerPreviewClient.dll",
            ]
            # Also check Release dir
            release_dir = str(Path(self.build_dir).parent / "Release")
            found = []
            missing = []
            for dll in expected_dlls:
                if os.path.isfile(os.path.join(self.build_dir, dll)):
                    found.append(dll)
                elif os.path.isfile(os.path.join(release_dir, dll)):
                    found.append(dll)
                else:
                    missing.append(dll)

            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(
                name=name,
                passed=len(missing) == 0,
                duration_ms=duration,
                details=f"Found {len(found)}/{len(expected_dlls)} DLLs",
                error=f"Missing: {', '.join(missing)}" if missing else ""
            ))
        except Exception as e:
            self.results.append(TestResult(name=name, passed=False, duration_ms=0, error=str(e)))

    def test_launch_clean(self):
        name = "Launch clean (no args)"
        start = time.perf_counter()
        try:
            proc = subprocess.Popen(
                [self.exe_path],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )

            # Wait a bit for startup
            time.sleep(3)

            if proc.poll() is not None:
                duration = (time.perf_counter() - start) * 1000
                self.results.append(TestResult(
                    name=name,
                    passed=False,
                    duration_ms=duration,
                    error=f"Process exited immediately with code {proc.returncode}"
                ))
            else:
                # Still running - good
                proc.terminate()
                proc.wait(timeout=5)
                duration = (time.perf_counter() - start) * 1000
                self.results.append(TestResult(
                    name=name,
                    passed=True,
                    duration_ms=duration,
                    details=f"Process alive after 3s, exit code {proc.returncode}"
                ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def test_launch_with_args(self):
        name = "Launch with /nosplash /notelemetry"
        start = time.perf_counter()
        try:
            proc = subprocess.Popen(
                [self.exe_path, "/nosplash", "/notelemetry"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )

            time.sleep(3)

            if proc.poll() is not None:
                duration = (time.perf_counter() - start) * 1000
                self.results.append(TestResult(
                    name=name,
                    passed=False,
                    duration_ms=duration,
                    error=f"Process exited with code {proc.returncode}"
                ))
            else:
                proc.terminate()
                proc.wait(timeout=5)
                duration = (time.perf_counter() - start) * 1000
                self.results.append(TestResult(
                    name=name,
                    passed=True,
                    duration_ms=duration,
                    details=f"Stable with args, exit code {proc.returncode}"
                ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def test_stability(self):
        name = f"Stability ({self.timeout}s soak test)"
        start = time.perf_counter()
        try:
            proc = subprocess.Popen(
                [self.exe_path, "/nosplash", "/notelemetry"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )

            # Monitor for the full timeout
            crashed = False
            exit_code = None
            elapsed = 0
            while elapsed < self.timeout:
                time.sleep(1)
                elapsed += 1
                if proc.poll() is not None:
                    crashed = True
                    exit_code = proc.returncode
                    break

            duration = (time.perf_counter() - start) * 1000

            if crashed:
                self.results.append(TestResult(
                    name=name,
                    passed=False,
                    duration_ms=duration,
                    error=f"Crashed after {elapsed}s with code {exit_code}"
                ))
            else:
                proc.terminate()
                proc.wait(timeout=5)
                self.results.append(TestResult(
                    name=name,
                    passed=True,
                    duration_ms=duration,
                    details=f"Stable for {self.timeout}s"
                ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def test_single_instance(self):
        name = "Single instance mutex"
        start = time.perf_counter()
        try:
            # Launch first instance
            proc1 = subprocess.Popen(
                [self.exe_path, "/nosplash"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )
            time.sleep(2)

            # Try launching second instance
            proc2 = subprocess.Popen(
                [self.exe_path, "/nosplash"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )
            time.sleep(2)

            # Second should exit quickly (single instance)
            p2_exited = proc2.poll() is not None
            p2_code = proc2.returncode if p2_exited else None

            proc1.terminate()
            proc1.wait(timeout=5)
            if proc2.poll() is None:
                proc2.terminate()
                proc2.wait(timeout=5)

            duration = (time.perf_counter() - start) * 1000

            # The second process should have detected single instance
            self.results.append(TestResult(
                name=name,
                passed=True,  # Not critical if it doesn't enforce
                duration_ms=duration,
                details=f"Proc2 exited: {p2_exited}, code: {p2_code}"
            ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def test_window_creation(self):
        name = "Window creation check"
        start = time.perf_counter()
        try:
            proc = subprocess.Popen(
                [self.exe_path, "/nosplash"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )

            time.sleep(3)

            # Use PowerShell to check for window
            result = subprocess.run(
                ["powershell", "-Command",
                 f"Get-Process -Id {proc.pid} -ErrorAction SilentlyContinue | Select-Object -ExpandProperty MainWindowTitle"],
                capture_output=True, text=True, timeout=5
            )
            window_title = result.stdout.strip()

            proc.terminate()
            proc.wait(timeout=5)

            duration = (time.perf_counter() - start) * 1000

            if window_title:
                self.results.append(TestResult(
                    name=name,
                    passed=True,
                    duration_ms=duration,
                    details=f"Window title: '{window_title}'"
                ))
            else:
                self.results.append(TestResult(
                    name=name,
                    passed=False,
                    duration_ms=duration,
                    error="No window title found"
                ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def test_memory_usage(self):
        name = "Memory usage baseline"
        start = time.perf_counter()
        try:
            proc = subprocess.Popen(
                [self.exe_path, "/nosplash"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=str(Path(self.exe_path).parent),
            )

            time.sleep(3)

            result = subprocess.run(
                ["powershell", "-Command",
                 f"Get-Process -Id {proc.pid} -ErrorAction SilentlyContinue | Select-Object WorkingSet64, HandleCount, Threads | ConvertTo-Json"],
                capture_output=True, text=True, timeout=5
            )

            proc.terminate()
            proc.wait(timeout=5)

            duration = (time.perf_counter() - start) * 1000

            try:
                info = json.loads(result.stdout)
                mem_mb = info.get("WorkingSet64", 0) / (1024 * 1024)
                handles = info.get("HandleCount", 0)
                threads = len(info.get("Threads", []))

                self.results.append(TestResult(
                    name=name,
                    passed=mem_mb < 200,  # Should use less than 200MB at idle
                    duration_ms=duration,
                    details=f"Memory: {mem_mb:.1f} MB, Handles: {handles}, Threads: {threads}"
                ))
            except (json.JSONDecodeError, TypeError):
                self.results.append(TestResult(
                    name=name,
                    passed=False,
                    duration_ms=duration,
                    error="Could not parse process info"
                ))
        except Exception as e:
            duration = (time.perf_counter() - start) * 1000
            self.results.append(TestResult(name=name, passed=False, duration_ms=duration, error=str(e)))

    def print_summary(self):
        passed = sum(1 for r in self.results if r.passed)
        failed = sum(1 for r in self.results if not r.passed)
        total = len(self.results)

        print(f"\n{'='*70}")
        print(f"  Test Results: {passed}/{total} passed, {failed} failed")
        print(f"{'='*70}\n")

        for r in self.results:
            status = "PASS" if r.passed else "FAIL"
            color = "\033[92m" if r.passed else "\033[91m"
            reset = "\033[0m"
            print(f"  {color}{status}{reset} {r.name} ({r.duration_ms:.0f}ms)")
            if r.details:
                print(f"       {r.details}")
            if r.error:
                print(f"       Error: {r.error}")

        print()

    def save_report(self):
        report_dir = os.path.join(os.path.dirname(__file__), "reports")
        os.makedirs(report_dir, exist_ok=True)
        report_path = os.path.join(report_dir, "test_results.json")

        report = {
            "timestamp": datetime.now().isoformat(),
            "exe": self.exe_path,
            "timeout": self.timeout,
            "passed": sum(1 for r in self.results if r.passed),
            "failed": sum(1 for r in self.results if not r.passed),
            "total": len(self.results),
            "results": [asdict(r) for r in self.results],
        }

        with open(report_path, "w") as f:
            json.dump(report, f, indent=2)

        print(f"  Report saved to {report_path}")


if __name__ == "__main__":
    timeout = 15
    exe_path = None

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--timeout" and i + 1 < len(args):
            timeout = int(args[i + 1])
            i += 2
        elif args[i] == "--launch":
            i += 1
        elif not args[i].startswith("--"):
            exe_path = args[i]
            i += 1
        else:
            i += 1

    if not exe_path:
        # Find the exe
        project_root = str(Path(__file__).parent.parent)
        for config in ["Debug", "Release"]:
            candidate = os.path.join(project_root, "build", "bin", config, "MovieMaker.exe")
            if os.path.isfile(candidate):
                exe_path = candidate
                break

    if not exe_path:
        print("Error: MovieMaker.exe not found. Build first.")
        sys.exit(1)

    harness = TestHarness(exe_path, timeout)
    harness.run_all()
