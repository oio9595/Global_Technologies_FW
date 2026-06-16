import re
from pathlib import Path

# prebuild.py가 있는 폴더
script_dir = Path(__file__).resolve().parent

# version.h 경로
version_h = script_dir.parent / "App" / "user" / "Inc" / "version.h"
# print(version_h)

with open(version_h, "r") as f:
    text = f.read()

m = re.search(r'#define\s+FW_BUILD\s+(\d+)', text)
num = int(m.group(1)) + 1

text = re.sub(
    r'(#define\s+FW_BUILD\s+)(\d+)',
    rf'\g<1>{num}',
    text
)

with open(version_h, "w") as f:
    f.write(text)

print(f"FW_BUILD has been updated to {num}.")