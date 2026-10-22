"""Install in FW/Build; invoke only after successful binary generation.

Requires version.h to include git_version.h. Release builds must use Rebuild All.
Generated header and build outputs must be ignored by Git.
"""
import json
import re
import shutil
import subprocess
from pathlib import Path

FW = Path(__file__).resolve().parent.parent
STATE = FW / "App/EWARM/App/build_state.json"
HEADER = FW / "App/User/App/Inc/git_version.h"
VERSION = FW / "App/User/App/Inc/version.h"
SOURCE = FW / "App/EWARM/App/Exe/App.bin"
RELEASE = SOURCE.parent / "Release"
FIRMWARE_FILE_PREFIX = "ID601"


def git(*args):
    return subprocess.run(["git", *args], cwd=FW, check=True,
                          capture_output=True, text=True).stdout.strip()


def skip(reason):
    print(f"[RELEASE SKIPPED] {reason}")
    return 0


def main():
    try:
        state = json.loads(STATE.read_text(encoding="utf-8"))
        if not isinstance(state, dict):
            return skip("Invalid build information.")
    except (OSError, ValueError):
        return skip("Missing or invalid build information; run prebuild first.")
    finally:
        # One attempt per prebuild; a manual repeat cannot reuse the record.
        STATE.unlink(missing_ok=True)

    if state.get("git_check_ok") is not True:
        return skip("Git inspection failed at build start.")
    if state.get("clean") is not True:
        return skip("Uncommitted changes existed at build start.")
    commit = state.get("commit", "")
    build_id = state.get("build_id", "")
    if not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", commit):
        return skip("Invalid commit information.")
    if not isinstance(build_id, str) or not re.fullmatch(r"[0-9a-f]{32}", build_id):
        return skip("Invalid build ID.")
    try:
        if git("rev-parse", "HEAD") != commit:
            return skip("HEAD changed during the build.")
        if git("status", "--porcelain=v1", "--untracked-files=all"):
            return skip("Working tree is dirty after the build.")
        if git("rev-parse", "HEAD") != commit:
            return skip("HEAD changed during final inspection.")
    except (OSError, subprocess.SubprocessError) as error:
        return skip(f"Final Git inspection failed: {error}")

    header = HEADER.read_text(encoding="utf-8")
    expected_revision = commit[:12]
    if not re.search(r'^#define FW_GIT_REV "' + expected_revision + r'"$', header, re.M):
        return skip("Generated Git revision does not match this build.")
    if f'#define FW_BUILD_ID "{build_id}"' not in header:
        return skip("Generated header belongs to another build.")
    if not SOURCE.is_file():
        raise RuntimeError(f"Binary not found: {SOURCE}")
    text = VERSION.read_text(encoding="utf-8")
    numbers = []
    for name in ("FW_VER_MAJOR", "FW_VER_MINOR", "FW_VER_BUILD"):
        match = re.search(r"^\s*#define\s+" + name + r"\s+(\d+)\s*$", text, re.M)
        if not match:
            raise RuntimeError(f"Invalid version macro: {name}")
        numbers.append(match.group(1))
    # cli.c prints FW_GIT_REV. Require that revision to exist in the image.
    if expected_revision.encode("ascii") not in SOURCE.read_bytes():
        raise RuntimeError("Binary does not contain the expected Git revision; Rebuild All required.")
    destination = RELEASE / (f'{FIRMWARE_FILE_PREFIX}_v{".".join(numbers)}_{expected_revision}.bin')
    RELEASE.mkdir(parents=True, exist_ok=True)
    shutil.copy2(SOURCE, destination)
    print(f"[RELEASE CREATED] {destination}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"[ERROR] {error}")
        raise SystemExit(1)
