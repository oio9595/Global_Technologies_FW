import re
import subprocess
import sys
from pathlib import Path

# 1. 명령행 인자 분석 (--minor, --major 확인)
is_minor_up = "--minor" in sys.argv
is_major_up = "--major" in sys.argv

# 2. 경로 정의
script_dir = Path(__file__).resolve().parent

# 상위 폴더로 거슬러 올라가며 .git 폴더 찾기
git_root_dir = None
for parent in [script_dir] + list(script_dir.parents):
    if (parent / ".git").is_dir():
        git_root_dir = parent
        break

if git_root_dir is None:
    print("Warning: .git directory not found in any parent folders. Using script_dir.")
    git_root_dir = script_dir

version_h = script_dir.parent / "App" / "user" / "Inc" / "version.h"

# 3. Git 최신 리비전 번호 및 수정 사항(-dirty) 감지
try:
    # 3-1. 최신 커밋 해시 가져오기
    git_rev = (
        subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=git_root_dir
        )
        .decode("utf-8")
        .strip()
    )

    # 3-2. version.h를 제외한 다른 파일의 변경 사항 감지
    try:
        rel_version_h = version_h.relative_to(git_root_dir).as_posix()
        status_cmd = ["git", "status", "--porcelain", "--", f":!{rel_version_h}"]
    except Exception:
        status_cmd = ["git", "status", "--porcelain"]

    git_status = (
        subprocess.check_output(status_cmd, cwd=git_root_dir)
        .decode("utf-8")
        .strip()
    )

    print(f"Git revision detected: {git_rev}")

    # ================= [디버깅 메시지 추가] =================
    if git_status:
        print("====== [DEBUG] Git Dirty Detected Files ======")
        print(git_status)
        print("=============================================")
        git_rev += "-dirty"
    else:
        print("[DEBUG] Git status is completely clean!")
    # ========================================================

except Exception as e:
    print(f"Warning: Failed to get git revision ({e}). Using 'unknown'")
    git_rev = "unknown"


# 4. version.h 파일 읽기
with open(version_h, "r", encoding="utf-8") as f:
    text = f.read()


# 5. 버전 제어 핵심 로직
if is_major_up:
    # --- MAJOR 버전 업인 경우 ---
    major_match = re.search(r"#define\s+FW_MAJOR\s+(\d+)", text)
    if major_match:
        new_major = int(major_match.group(1)) + 1
        text = re.sub(r"(#define\s+FW_MAJOR\s+)(\d+)", rf"\g<1>{new_major}", text)
        print(f"[MAJOR UP] FW_MAJOR has been bumped to {new_major}.")
    else:
        print("Warning: FW_MAJOR not found in version.h")

    # MAJOR가 올라가면 MINOR와 BUILD는 모두 0으로 리셋하는 것이 일반적입니다.
    if re.search(r"#define\s+FW_MINOR\s+\d+", text):
        text = re.sub(r"(#define\s+FW_MINOR\s+)(\d+)", r"\g<1>0", text)
        print("[MAJOR UP] FW_MINOR has been reset to 0.")

    if re.search(r"#define\s+FW_BUILD\s+\d+", text):
        text = re.sub(r"(#define\s+FW_BUILD\s+)(\d+)", r"\g<1>0", text)
        print("[MAJOR UP] FW_BUILD has been reset to 0.")

elif is_minor_up:
    # --- MINOR 버전 업인 경우 ---
    minor_match = re.search(r"#define\s+FW_MINOR\s+(\d+)", text)
    if minor_match:
        new_minor = int(minor_match.group(1)) + 1
        text = re.sub(r"(#define\s+FW_MINOR\s+)(\d+)", rf"\g<1>{new_minor}", text)
        print(f"[MINOR UP] FW_MINOR has been bumped to {new_minor}.")
    else:
        print("Warning: FW_MINOR not found in version.h")

    # MINOR가 올라가면 BUILD는 0으로 리셋
    if re.search(r"#define\s+FW_BUILD\s+\d+", text):
        text = re.sub(r"(#define\s+FW_BUILD\s+)(\d+)", r"\g<1>0", text)
        print("[MINOR UP] FW_BUILD has been reset to 0.")

else:
    # --- 일반 모드 (아무 인자도 없을 때) ---
    build_match = re.search(r"#define\s+FW_BUILD\s+(\d+)", text)
    if build_match:
        new_build = int(build_match.group(1)) + 1
        text = re.sub(r"(#define\s+FW_BUILD\s+)(\d+)", rf"\g<1>{new_build}", text)
        print(f"[NORMAL MODE] FW_BUILD has been updated to {new_build}.")
    else:
        print("Warning: FW_BUILD not found in version.h")

# 6. FW_GIT_REV 값 업데이트하기
if re.search(r"#define\s+FW_GIT_REV\s+", text):
    # 이 줄이 핵심입니다. 기존 줄에 -dirty가 있든 말든 그 줄 전체(.*)를 밀어버립니다.
    text = re.sub(r'#define\s+FW_GIT_REV\s+.*', f'#define FW_GIT_REV "{git_rev}"', text)
else:
    text += f'\n#define FW_GIT_REV "{git_rev}"\n'

print(f"FW_GIT_REV has been updated to \"{git_rev}\".")

# 7. 파일 저장
with open(version_h, "w", encoding="utf-8") as f:
    f.write(text)