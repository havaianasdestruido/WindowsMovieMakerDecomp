"""Optional independent pefile oracle for the synthetic fixtures (#26, #27).

Install pefile in an isolated venv, then run this script with that interpreter.
The regular unittest suite deliberately has no external Python dependencies.
"""
from pathlib import Path
import sys
import tempfile
import pefile

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools'))
import diff_exports
from test_tools import fixture

for plus in (False, True):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / 'fixture.dll'
        path.write_bytes(fixture(plus=plus))
        pe = pefile.PE(str(path))
        exports = {(e.name.decode() if e.name else '#' + str(e.ordinal)): e.ordinal
                   for e in pe.DIRECTORY_ENTRY_EXPORT.symbols}
        imports = {i.dll.decode().lower() for i in pe.DIRECTORY_ENTRY_IMPORT}
        assert diff_exports.parse_exports(path) == exports
        assert diff_exports.parse_imports(path) == imports
        pe.close()
print('pefile oracle: PE32 and PE32+ export/import tables match')
