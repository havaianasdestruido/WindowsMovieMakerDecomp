"""Synthetic, copyright-free PE fixtures and tool regressions (#26–#31)."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import diff_exports as diff
import pe_analyzer
import test_all as runtime


def fixture(plus=False, export_name=b'Foo', ordinal=7, empty=False):
    data = bytearray(2048)
    def put(off, fmt, *values):
        struct.pack_into(fmt, data, off, *values)
    data[:2] = b'MZ'
    put(0x3c, '<I', 0x80)
    data[0x80:0x84] = b'PE\0\0'
    opt_size = 240 if plus else 224
    put(0x84, '<HHIIIHH', 0x8664 if plus else 0x14c, 1, 0, 0, 0, opt_size, 0x2102)
    opt = 0x98
    put(opt, '<H', 0x20b if plus else 0x10b)
    put(opt + 16, '<I', 0x1200)
    if plus:
        put(opt + 24, '<Q', 0x140000000)
    else:
        put(opt + 24, '<II', 0x3000, 0x400000)  # BaseOfData != ImageBase (#27)
    put(opt + 60, '<I', 512)
    dirs = opt + (112 if plus else 96)
    put(dirs - 4, '<I', 16)
    if not empty:
        put(dirs, '<IIII', 0x1000, 0x100, 0x1100, 60)
    sec = opt + opt_size
    data[sec:sec + 8] = b'.rdata\0\0'
    put(sec + 8, '<IIII', 1536, 0x1000, 1536, 512)
    put(sec + 36, '<I', 0x40000040)
    # Base, functions, names, EAT, name RVAs, ordinals. Two exports: named + ordinal-only.
    put(512 + 16, '<IIIIII', ordinal, 3, 1, 0x1040, 0x1050, 0x1060)
    put(512 + 0x40, '<III', 0x1200, 0, 0x1210)  # hole at index 1
    put(512 + 0x50, '<I', 0x1070)
    put(512 + 0x60, '<H', 0)
    data[512 + 0x70:512 + 0x70 + len(export_name) + 1] = export_name + b'\0'
    put(768, '<IIIII', 0, 0, 0, 0x1150, 0x1180)
    put(788, '<IIIII', 0, 0, 0, 0x1160, 0x1190)
    data[848:861] = b'KERNEL32.dll\0'
    data[864:875] = b'USER32.dll\0'
    thunk_format = '<QQ' if plus else '<II'
    ordinal_flag = 1 << (63 if plus else 31)
    put(896, thunk_format, ordinal_flag | 1, 0)
    put(912, thunk_format, ordinal_flag | 2, 0)
    return data


class PETools(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.file = Path(self.tmp.name) / 'test.dll'
        self.file.write_bytes(fixture())

    def test_pe32_tables(self):
        self.assertEqual(diff.parse_exports(self.file), {'Foo': 7, '#9': 9})
        self.assertEqual(diff.parse_imports(self.file), {'kernel32.dll', 'user32.dll'})

    def test_pe32plus_tables(self):
        self.file.write_bytes(fixture(plus=True))
        self.test_pe32_tables()

    def test_image_bases(self):
        for plus, expected in [(False, 0x400000), (True, 0x140000000)]:
            self.file.write_bytes(fixture(plus=plus))
            self.assertEqual(pe_analyzer.parse_pe(str(self.file)).image_base, expected)

    def test_legitimate_empty_directories(self):
        self.file.write_bytes(fixture(empty=True))
        self.assertEqual(diff.parse_exports(self.file), {})
        self.assertEqual(diff.parse_imports(self.file), set())

    def test_truncated_input_is_error(self):
        for length in (0, 2, 100, 300, 1000):
            self.file.write_bytes(fixture()[:length])
            with self.assertRaises((ValueError, struct.error)):
                diff.parse_exports(self.file)

    def test_bad_signature_is_error(self):
        data = fixture()
        data[0x80] = 0
        self.file.write_bytes(data)
        with self.assertRaises(diff.PEFormatError):
            diff.parse_exports(self.file)

    def test_invalid_ordinal_is_error(self):
        data = fixture()
        struct.pack_into('<H', data, 512 + 0x60, 3)
        self.file.write_bytes(data)
        with self.assertRaises(diff.PEFormatError):
            diff.parse_exports(self.file)

    def test_unterminated_imports_are_error(self):
        data = fixture()
        struct.pack_into('<I', data, 0x98 + 96 + 12, 40)
        self.file.write_bytes(data)
        with self.assertRaises(diff.PEFormatError):
            diff.parse_imports(self.file)

    def run_diff(self, *args):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return diff.main(list(map(str, args)))

    def test_cli_missing_and_malformed_fail(self):
        self.assertEqual(self.run_diff(self.file, self.file.with_name('missing.dll')), 2)
        self.file.write_bytes(b'bad')
        self.assertEqual(self.run_diff(self.file, self.file), 2)

    def test_cli_parity_and_report_mode(self):
        other = self.file.with_name('other.dll')
        other.write_bytes(fixture(export_name=b'Bar'))
        self.assertEqual(self.run_diff(self.file, self.file), 0)
        self.assertEqual(self.run_diff(self.file, other), 1)
        self.assertEqual(self.run_diff('--exit-zero', self.file, other), 0)
        other.write_bytes(fixture(ordinal=8))
        self.assertEqual(self.run_diff(self.file, other), 1)

    def test_batch_unpaired_and_empty_fail(self):
        empty = Path(self.tmp.name) / 'empty'
        empty.mkdir()
        self.assertEqual(self.run_diff('--batch', self.file.parent, empty), 2)
        (empty / 'other.dll').write_bytes(fixture())
        self.assertEqual(self.run_diff('--batch', self.file.parent, empty), 1)


class RuntimeTools(unittest.TestCase):
    def test_dlls_next_to_exe_in_any_configuration(self):
        for config in ('Debug', 'Release', 'custom'):
            with tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp) / 'build' / 'bin' / config
                folder.mkdir(parents=True)
                exe = folder / 'MovieMaker.exe'
                exe.touch()
                # Populate exactly the manifest the production check uses.
                names = ['MovieMakerCore', 'WLXPhotoBase', 'WLXPipeline', 'WLXPipetran',
                         'WLXSlideshow', 'WLXVideoTrim', 'WLXPhotoCinematic', 'WLXMovieLibrary',
                         'WLXMP4Parser', 'WLXFaceRecognition', 'WLXMediaPublishSubscribe',
                         'WLMFDS', 'WLMFReadWrite', 'MovieMakerLang', 'MovieMakerPreviewClient']
                for name in names:
                    (folder / (name + '.dll')).touch()
                harness = runtime.TestHarness(str(exe))
                harness.test_dlls_present()
                self.assertTrue(harness.results[0].passed, harness.results[0].error)

    def test_missing_exe_cli_fails(self):
        proc = subprocess.run([sys.executable, str(ROOT / 'tools/test_all.py'),
                               '/nonexistent/MovieMaker.exe', '--timeout', '0'], capture_output=True)
        self.assertEqual(proc.returncode, 1)
        self.assertIn(b'not found', proc.stderr)

    def test_launch_only_dispatch_and_status(self):
        harness = runtime.TestHarness('unused')
        def launch():
            harness.results.append(runtime.TestResult('launch', False, 0))
        with patch.object(harness, 'test_launch_clean', side_effect=launch) as called, \
             patch.object(harness, 'test_dlls_present') as dlls, \
             patch.object(harness, 'save_report'), patch.object(harness, 'print_summary'), \
             contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(harness.run_all(launch_only=True), 1)
            called.assert_called_once()
            dlls.assert_not_called()

    def test_launch_flag_reaches_harness(self):
        with tempfile.NamedTemporaryFile() as exe, patch.object(runtime.TestHarness, 'run_all', return_value=0) as run:
            self.assertEqual(runtime.main([exe.name, '--launch']), 0)
            run.assert_called_once_with(launch_only=True)

    def test_build_script_root_and_generator(self):
        source = (ROOT / 'tools/build.ps1').read_text()
        self.assertIn('$ProjectRoot = Split-Path -Parent $PSScriptRoot', source)
        self.assertNotIn('Split-Path -Parent (Split-Path', source)
        self.assertIn('-G "Visual Studio 17 2022" -A Win32', source)


if __name__ == '__main__':
    unittest.main()
