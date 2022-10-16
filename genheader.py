#!/usr/bin/env python3
# SPDX-License-Identifier: ISC

# converts a .asm file to the corresponding .h file

import subprocess
import os
import sys

if len(sys.argv) != 2:
    print("Usage: genheader.py <asm file>")

inp = sys.argv[1]
outf = inp.removesuffix(".asm") + ".h"
tmpf = inp + ".bin"
subprocess.run(["nasm", inp, "-f", "bin", "-o", tmpf]).check_returncode()

data = open(tmpf, 'rb').read()
sym = open("header_sym.map", 'r').read()
out = open(outf, 'w')

out.write("static const byte header_bin[] = {\n")
for i in range(0, len(data), 16):
    blk = data[i:i+16]
    line = "".join(f"0x{x:02x}," for x in blk)
    out.write("  " + line + "\n")
out.write("};\n")

for x in sym.splitlines():
    if x.split() and x.split()[-1].startswith("export_"):
        val = int(x.split()[0], 16)
        name = x.split()[-1].replace("export_", "header_", 1)
        out.write(f"const int {name} = 0x{val:x};\n")

if True:
    os.unlink(tmpf)
    os.unlink("header_sym.map")
