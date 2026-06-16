import re
import subprocess
from pathlib import Path

# prebuild.py가 있는 폴더
script_dir = Path(__file__).resolve().parent

# [자동화 핵심] 상위 폴더로 거슬러 올라가며 .git 폴더 찾기
git_root_dir = None
for parent in [script_dir] + list(script_dir.parents):
    if (parent / ".git").is_dir():
        git_root_dir = parent
        break

# 만약 끝까지 올라갔는데도 .git을 못 찾았다면 예외 처리
if git_root_dir is None:
    print("Warning: .git directory not found in any parent folders. Using script_dir.")
    git_root_dir = script_dir

# version.h 경로
version_h = script_dir.parent / "App" / "user" / "Inc" / "version.h"

# print(version_h)# 2. Git 최신 리비전 번호(7자리 숏해시) 가져오기
try:
    # 자동으로 찾은 git_root_dir 위치에서 명령어를 실행합니다.
    git_rev = (
        subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=git_root_dir
        )
        .decode("utf-8")
        .strip()
    )
except Exception as e:
    print(f"Warning: Failed to get git revision ({e}). Using 'unknown'")
    git_rev = "unknown"

with open(version_h, "r") as f:
    text = f.read()

# 4. FW_BUILD 값 1 증가시키기
m = re.search(r"#define\s+FW_BUILD\s+(\d+)", text)
if m:
    num = int(m.group(1)) + 1
    text = re.sub(r"(#define\s+FW_BUILD\s+)(\d+)", rf"\g<1>{num}", text)
    print(f"FW_BUILD has been updated to {num}.")
else:
    print("Warning: FW_BUILD not found in version.h")
    num = "unknown"

# 5. FW_GIT_REV 값 업데이트하기
if re.search(r"#define\s+FW_GIT_REV\s+", text):
    text = re.sub(r'(#define\s+FW_GIT_REV\s+)("[^"]*")', rf'\g<1>"{git_rev}"', text)
else:
    text += f'\n#define FW_GIT_REV "{git_rev}"\n'

print(f"FW_GIT_REV has been updated to \"{git_rev}\".")

with open(version_h, "w") as f:
    f.write(text)

print(f"FW_BUILD has been updated to {num}.")