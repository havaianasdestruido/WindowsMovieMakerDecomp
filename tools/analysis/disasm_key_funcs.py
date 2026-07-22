#!/usr/bin/env python3
"""
Disassemble critical functions from MovieMakerCore.dll
Uses pefile + capstone for x86 disassembly
"""
import sys
import pefile
from capstone import *
from collections import OrderedDict

DLL_PATH = r"C:\Users\mcmco\Desktop\WMMR\undecomp\Photo Gallery\MovieMakerCore.dll"

def load_pe(path):
    pe = pefile.PE(path, fast_load=False)
    return pe

def get_export_map(pe):
    """Return {ordinal: (name, rva)} for all exports."""
    exports = {}
    if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        pe.parse_data_directories(directories=[pe.DIRECTORY_ENTRY_EXPORT])
    for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        ordinal = exp.ordinal
        name = exp.name.decode() if exp.name else f"ordinal_{ordinal}"
        rva = exp.address
        exports[ordinal] = (name, rva)
    return exports

def get_code_bytes(pe, rva, size):
    """Get raw bytes at RVA."""
    try:
        offset = pe.get_offset_from_rva(rva)
        return pe.get_data(offset, size)
    except Exception:
        return None

def disasm_slice(pe, rva, md, max_insns, label=""):
    """Disassemble up to max_insns starting at rva. Returns list of (addr, mnemonic, op_str)."""
    # Estimate size: average ~5 bytes per instruction, take generous slice
    size = max_insns * 8
    raw = get_code_bytes(pe, rva, size)
    if raw is None:
        print(f"  [!] Cannot read bytes at RVA 0x{rva:08X} for {label}")
        return []
    base = pe.OPTIONAL_HEADER.ImageBase
    instructions = list(md.disasm(raw, base + rva))
    return instructions[:max_insns]

def analyze_function(insns, label, pe):
    """Analyze disassembled instructions for patterns."""
    print(f"\n{'='*80}")
    print(f"  FUNCTION: {label}")
    print(f"  First address: 0x{insns[0].address:08X}")
    print(f"  Instruction count: {len(insns)}")
    print(f"{'='*80}")

    # Detect prologue
    prologues = {
        ('push', 'ebp'): 'push ebp; mov ebp, esp',
        ('push', 'ebx'): 'push ebx; mov ebx, esp',
        ('push', 'esi'): 'push esi; ...',
    }
    if len(insns) >= 2:
        combo = (insns[0].mnemonic, insns[0].op_str.split(',')[0].strip())
        if combo in prologues:
            # Check if next instruction is mov ebp/ebx, esp
            if insns[1].mnemonic == 'mov' and 'esp' in insns[1].op_str:
                print(f"  [PROLOGUE] {prologues[combo]} + {insns[1].mnemonic} {insns[1].op_str}")
            else:
                print(f"  [PROLOGUE] {insns[0].mnemonic} {insns[0].op_str}")

    # Collect stats
    call_targets = []
    string_refs = []
    indirect_calls = []
    stack_args = []
    com_patterns = []

    for i, insn in enumerate(insns):
        # Call targets
        if insn.mnemonic == 'call':
            target = insn.op_str
            if target.startswith('0x') or (target[0].isdigit() == False and '[' not in target):
                call_targets.append((insn.address, target))

        # Indirect calls (call [disp32]) - through IAT/vtable
        if insn.mnemonic == 'call' and '[' in insn.op_str:
            indirect_calls.append((insn.address, insn.op_str))

        # String references via lea
        if insn.mnemonic == 'lea' and '[' in insn.op_str:
            # Check if op has a numeric displacement that might be a string
            string_refs.append((insn.address, insn.op_str))

        # push instructions before a call (stack params) - look in windows around calls
        if insn.mnemonic == 'push':
            stack_args.append((insn.address, insn.op_str))

        # COM vtable: mov eax, [ecx+X] followed by call [eax+Y]
        if insn.mnemonic == 'mov' and 'eax' in insn.op_str and 'ecx' in insn.op_str:
            if i + 1 < len(insns) and insns[i+1].mnemonic == 'call' and 'eax' in insns[i+1].op_str:
                com_patterns.append((insn.address, f"{insn.mnemonic} {insn.op_str} -> {insns[i+1].mnemonic} {insns[i+1].op_str}"))

    # Print summary
    print(f"\n  --- Call Targets ({len(call_targets)}) ---")
    for addr, target in call_targets[:50]:
        print(f"    0x{addr:08X}: call {target}")

    print(f"\n  --- Indirect Calls (call [disp32]) ({len(indirect_calls)}) ---")
    for addr, target in indirect_calls:
        print(f"    0x{addr:08X}: call {target}")

    print(f"\n  --- COM VTable Patterns ({len(com_patterns)}) ---")
    for addr, pattern in com_patterns:
        print(f"    0x{addr:08X}: {pattern}")

    print(f"\n  --- LEA / String Reference Candidates ({len(string_refs)}) ---")
    for addr, ref in string_refs[:30]:
        print(f"    0x{addr:08X}: lea {ref}")

    return call_targets, indirect_calls, string_refs, com_patterns

def dump_indirect_calls_in_range(pe, rva_start, count, md):
    """Search through instructions and dump all call [disp32] patterns."""
    base = pe.OPTIONAL_HEADER.ImageBase
    size = count * 8
    raw = get_code_bytes(pe, rva_start, size)
    if raw is None:
        return []
    results = []
    for insn in md.disasm(raw, base + rva_start):
        if insn.mnemonic == 'call' and '[' in insn.op_str:
            results.append((insn.address, insn.op_str))
        if len(results) >= 200:
            break
    return results

def dump_string_refs(pe, rva_start, count, md):
    """Find all lea instructions that might reference strings."""
    base = pe.OPTIONAL_HEADER.ImageBase
    size = count * 8
    raw = get_code_bytes(pe, rva_start, size)
    if raw is None:
        return []
    results = []
    for insn in md.disasm(raw, base + rva_start):
        if insn.mnemonic in ('lea', 'mov') and 'offset' in insn.op_str.lower():
            results.append((insn.address, insn.mnemonic, insn.op_str))
        # Also catch push offset patterns
        if insn.mnemonic == 'push' and 'offset' in insn.op_str.lower():
            results.append((insn.address, insn.mnemonic, insn.op_str))
    return results

def try_read_string_at_rva(pe, rva, max_len=200):
    """Try to read a null-terminated string at RVA."""
    try:
        raw = get_code_bytes(pe, rva, max_len)
        if raw is None:
            return None
        end = raw.find(b'\x00')
        if end > 0:
            s = raw[:end]
            # Check if printable
            try:
                decoded = s.decode('ascii', errors='replace')
                if all(c.isprintable() or c in '\n\r\t' for c in decoded) and len(decoded) > 1:
                    return decoded
            except:
                pass
    except:
        pass
    return None

def main():
    print(f"Loading PE: {DLL_PATH}")
    pe = load_pe(DLL_PATH)

    print(f"  Machine: 0x{pe.FILE_HEADER.Machine:04X}")
    print(f"  ImageBase: 0x{pe.OPTIONAL_HEADER.ImageBase:08X}")
    print(f"  EntryPoint RVA: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08X}")
    print(f"  Subsystem: {pe.OPTIONAL_HEADER.Subsystem}")
    print(f"  Sections:")
    for sec in pe.sections:
        name = sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        print(f"    {name:8s} VA=0x{sec.VirtualAddress:08X} Size=0x{sec.Misc_VirtualSize:08X} Raw=0x{sec.SizeOfRawData:08X}")

    # Setup capstone for x86-32
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True

    # Get exports
    exports = get_export_map(pe)
    print(f"\nExports ({len(exports)}):")
    for ordinal in sorted(exports.keys()):
        name, rva = exports[ordinal]
        print(f"  Ordinal {ordinal}: {name} @ RVA 0x{rva:08X} (VA 0x{pe.OPTIONAL_HEADER.ImageBase + rva:08X})")

    # === 1. DllMain entry point ===
    ep_rva = pe.OPTIONAL_HEADER.AddressOfEntryPoint
    print(f"\n{'#'*80}")
    print(f"# 1. DISASSEMBLING ENTRY POINT (DllMain) at RVA 0x{ep_rva:08X}")
    print(f"{'#'*80}")
    ep_insns = disasm_slice(pe, ep_rva, md, 500, "DllMain")
    if ep_insns:
        for insn in ep_insns:
            print(f"  0x{insn.address:08X}: {insn.mnemonic:8s} {insn.op_str}")
        analyze_function(ep_insns, f"DllMain (RVA 0x{ep_rva:08X})", pe)

    # === 2. MovieMakerMain export ===
    mm_rva = None
    for ordinal, (name, rva) in exports.items():
        if 'MovieMakerMain' in name or 'main' in name.lower():
            mm_rva = rva
            mm_name = name
            break
    if mm_rva is None and exports:
        # Fall back to ordinal 1 or first export
        first_ord = min(exports.keys())
        mm_rva = exports[first_ord][1]
        mm_name = exports[first_ord][0]

    if mm_rva:
        print(f"\n{'#'*80}")
        print(f"# 2. DISASSEMBLING '{mm_name}' export at RVA 0x{mm_rva:08X}")
        print(f"{'#'*80}")
        mm_insns = disasm_slice(pe, mm_rva, md, 1000, mm_name)
        if mm_insns:
            for insn in mm_insns:
                print(f"  0x{insn.address:08X}: {insn.mnemonic:8s} {insn.op_str}")
            analyze_function(mm_insns, f"{mm_name} (RVA 0x{mm_rva:08X})", pe)
    else:
        print("\n[!] No MovieMakerMain export found")

    # === 3. Exported function stubs ===
    # Check ALL exports thoroughly
    other_exports = [(ordinal, name, rva) for ordinal, (name, rva) in exports.items()
                     if rva != mm_rva]
    stubs_to_disasm = other_exports[:50]

    if stubs_to_disasm:
        print(f"\n{'#'*80}")
        print(f"# 3. DISASSEMBLING {len(stubs_to_disasm)} EXPORT STUBS")
        print(f"{'#'*80}")
        for ordinal, name, rva in stubs_to_disasm:
            if rva == 0:
                print(f"\n  [SKIP] {name} (ordinal {ordinal}) - RVA=0x0 (likely forwarded or null)")
                continue
            print(f"\n  --- Export: {name} (ordinal {ordinal}) RVA 0x{rva:08X} ---")
            insns = disasm_slice(pe, rva, md, 100, name)
            if insns:
                for insn in insns:
                    print(f"    0x{insn.address:08X}: {insn.mnemonic:8s} {insn.op_str}")
                analyze_function(insns, f"{name} (ordinal {ordinal})", pe)
    else:
        print(f"\n  Only {len(exports)} export(s) found - no additional stubs beyond {mm_name}")

    # === 5. Dump all indirect calls in first section ===
    print(f"\n{'#'*80}")
    print(f"# 5. ALL INDIRECT CALLS (call [disp32]) in code section")
    print(f"{'#'*80}")

    # Use first executable section
    code_sec = None
    for sec in pe.sections:
        if sec.Characteristics & 0x20000000:  # IMAGE_SCN_MEM_EXECUTE
            code_sec = sec
            break
    if code_sec:
        sec_name = code_sec.Name.rstrip(b'\x00').decode('ascii', errors='replace')
        print(f"  Section: {sec_name} RVA 0x{code_sec.VirtualAddress:08X}")
        indirect = dump_indirect_calls_in_range(pe, code_sec.VirtualAddress, 5000, md)
        print(f"  Found {len(indirect)} indirect calls:")
        for addr, target in indirect:
            print(f"    0x{addr:08X}: call {target}")

    # === 6. String references ===
    print(f"\n{'#'*80}")
    print(f"# 6. STRING REFERENCES in first 5000 instructions")
    print(f"{'#'*80}")
    if code_sec:
        str_refs = dump_string_refs(pe, code_sec.VirtualAddress, 5000, md)
        print(f"  Found {len(str_refs)} string reference candidates:")
        for addr, mnemonic, operand in str_refs:
            # Try to resolve the offset to a string
            # Extract numeric offset from operand like "dword ptr [0x10012345]" or "offset 0x10012345"
            resolved = ""
            for token in operand.replace(',', ' ').split():
                try:
                    val = int(token, 16)
                    base = pe.OPTIONAL_HEADER.ImageBase
                    if base <= val < base + pe.OPTIONAL_HEADER.SizeOfImage:
                        str_rva = val - base
                        s = try_read_string_at_rva(pe, str_rva)
                        if s:
                            resolved = f'  -> "{s[:120]}"'
                            break
                except:
                    continue
            print(f"    0x{addr:08X}: {mnemonic:8s} {operand}{resolved}")

    # === Additional: dump disasm around entry point with extra context ===
    print(f"\n{'#'*80}")
    print(f"# BONUS: Raw disassembly dump of first 200 bytes after entry point")
    print(f"{'#'*80}")
    raw = get_code_bytes(pe, ep_rva, 200)
    if raw:
        print(f"  Hex dump of code at RVA 0x{ep_rva:08X}:")
        for i in range(0, min(len(raw), 200), 16):
            hexpart = ' '.join(f'{b:02X}' for b in raw[i:i+16])
            print(f"    +{i:04X}: {hexpart}")

    pe.close()
    print(f"\n{'='*80}")
    print("DONE")
    print(f"{'='*80}")

if __name__ == '__main__':
    main()
