import re
import subprocess
from pathlib import Path

script_dir = Path(__file__).resolve().parent

git_root_dir = None
for parent in [script_dir] + list(script_dir.parents):
    if (parent / ".git").is_dir():
        git_root_dir = parent
        break

if git_root_dir is None:
    print("Warning: .git directory not found in any parent folders. Using script_dir.")
    git_root_dir = script_dir

version_h = script_dir.parent / "App" / "user" / "Inc" / "version.h"

try:
    git_rev = (
        subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=git_root_dir
        )
        .decode("utf-8")
        .strip()
    )

    git_status = (
            subprocess.check_output(
                ["git", "status", "--porcelain"],
                cwd=git_root_dir
            )
            .decode("utf-8")
            .strip()
        )
# 수정된 파일이 하나라도 있다면 해시 뒤에 '-dirty'를 붙임
    if git_status:
        git_rev += "-dirty"

except Exception as e:
    print(f"Warning: Failed to get git revision ({e}). Using 'unknown'")
    git_rev = "unknown"

with open(version_h, "r") as f:
    text = f.read()

m = re.search(r"#define\s+FW_BUILD\s+(\d+)", text)
if m:
    num = int(m.group(1)) + 1
    text = re.sub(r"(#define\s+FW_BUILD\s+)(\d+)", rf"\g<1>{num}", text)
    print(f"FW_BUILD has been updated to {num}.")
else:
    print("Warning: FW_BUILD not found in version.h")
    num = "unknown"

if re.search(r"#define\s+FW_GIT_REV\s+", text):
    text = re.sub(r'(#define\s+FW_GIT_REV\s+)("[^"]*")', rf'\g<1>"{git_rev}"', text)
else:
    text += f'\n#define FW_GIT_REV "{git_rev}"\n'

print(f"FW_GIT_REV has been updated to \"{git_rev}\".")

with open(version_h, "w") as f:
    f.write(text)