#!/usr/bin/env python3

import os
import subprocess
import re
import itertools
import argparse

# Constants
asm_addr_re = re.compile("^0x([a-f0-9]+):")
objdump_addr_line = re.compile("^([a-f0-9]+):")


boring = {
  "blank_prefix": " ",
  "hit_prefix": "+",
  "miss_prefix": "-",
  "postfix" : "",
}

color = {
  "blank_prefix": " ",
  "miss_prefix": "\x1b[31m+",
  "hit_prefix": "\x1b[32m+",
  "postfix": "\x1b[0m",
}

# Runtime
parser = argparse.ArgumentParser(description="Annotates the kernel with coverage information")
parser.add_argument('--source', action='store_true', help='annotate objdump -S')
parser.add_argument('--no-color', action='store_true', help='do not display colors')
args = parser.parse_args()

theme = color if not args.no_color else boring
objdump_flag = '-S' if args.source else '-d'

if not os.path.isfile("kernel/george.c"):
  print("This program must be called from the root of the SILVOS repo")
  exit(1)

if not os.path.isfile("george.multiboot"):
  print("Build the kernel first")
  exit(1)

try:
  coverage_files = subprocess.check_output("ls userland/*/coverage.log", shell=True, stderr=subprocess.PIPE).strip().split(b"\n")
except subprocess.CalledProcessError:
  print("Run `make test` to generate coverage information first")
  exit(1)

def get_covered_addrs(h):
  tbs = {}
  current_tb = None
  for line in list(h):
    if line.startswith("IN"): # start of TB translation
      current_tb = None
    else:
      m = asm_addr_re.match(line)
      addr = int(m.group(1), 16)
      if current_tb is None:
        current_tb = []
        tbs[addr] = current_tb
      current_tb.append(addr)
  return set(itertools.chain.from_iterable(tbs.values()))

all_addrs = set()
for cf in coverage_files:
  with open(cf, "r") as h:
    all_addrs |= get_covered_addrs(h)

lines = subprocess.check_output(["objdump", objdump_flag, "george.multiboot"]).decode("ascii").split("\n")
for line in lines:
  match = objdump_addr_line.match(line)
  if match is None:
    print(theme["blank_prefix"] + line + theme["postfix"])
    continue
  addr = int(match.group(1), 16)
  print((theme["hit_prefix"] if addr in all_addrs else theme["miss_prefix"])  + line + theme["postfix"])
