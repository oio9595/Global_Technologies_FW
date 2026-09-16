from pathlib import Path
import re
import shutil
import sys

FIRMWARE_FILE_PREFIX = "ID804_FA_Board"

version_header = Path(sys.argv[1])
source_bin = Path(sys.argv[2])
output_dir = Path(sys.argv[3])

text = version_header.read_text(encoding="utf-8")

def macro(name):
    match = re.search(rf"^\s*#define\s+{name}\s+(\d+)", text, re.MULTILINE)
    if not match:
        raise RuntimeError(f"{name} is not defined in {version_header}")
    return match.group(1)

major = macro("FW_VER_MAJOR")
minor = macro("FW_VER_MINOR")
build = macro("FW_VER_BUILD")

version = f"v{major}.{minor}.{build}"
output_dir.mkdir(parents=True, exist_ok=True)

destination = output_dir / f"{FIRMWARE_FILE_PREFIX}_{version}.bin"
shutil.copy2(source_bin, destination)

print(f"Created: {destination}")