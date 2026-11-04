"""Install in FW/Build. Git state never blocks development compilation."""
import json
import subprocess
import uuid
from pathlib import Path

FW = Path(__file__).resolve().parent.parent
STATE = FW / "App/EWARM/App/build_state.json"
HEADER = FW / "App/User/App/Inc/git_version.h"


def git(*args):
    return subprocess.run(["git", *args], cwd=FW, check=True,
                          capture_output=True, text=True).stdout.strip()


def main():
    STATE.parent.mkdir(parents=True, exist_ok=True)
    STATE.unlink(missing_ok=True)
    state = dict(git_check_ok=False, clean=False, commit="",
                 git_revision="unknown", build_id=uuid.uuid4().hex)
    try:
        git("rev-parse", "--show-toplevel")
        commit = git("rev-parse", "HEAD")
        dirty = bool(git("status", "--porcelain=v1", "--untracked-files=all"))
        if git("rev-parse", "HEAD") != commit:
            raise RuntimeError("HEAD changed during Git inspection")
        state.update(git_check_ok=True, clean=not dirty, commit=commit,
                     git_revision=commit[:12] + ("-dirty" if dirty else ""))
    except (OSError, subprocess.SubprocessError, RuntimeError) as error:
        print(f"[WARNING] Git inspection failed: {error}")

    # A unique ID forces the consumer to recompile and identifies this build.
    HEADER.write_text(
        "#ifndef GENERATED_GIT_VERSION_H\n#define GENERATED_GIT_VERSION_H\n"
        f'#define FW_GIT_REV "{state["git_revision"]}"\n'
        f'#define FW_BUILD_ID "{state["build_id"]}"\n'
        "#endif\n", encoding="utf-8")
    STATE.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    print(f'[BUILD] Git: {state["git_revision"]}; release eligible: {state["clean"]}')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
