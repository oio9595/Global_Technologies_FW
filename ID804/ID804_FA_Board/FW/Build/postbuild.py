from pathlib import Path
import argparse
import re
import shutil

FIRMWARE_FILE_PREFIX = "ID804_FA_Board"

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent

VERSION_HEADER = PROJECT_ROOT / "App" / "User" / "App" / "Inc" / "version.h"
SOURCE_BIN = PROJECT_ROOT / "App" / "EWARM" / "App" / "Exe" / "App.bin"
RELEASE_DIR = PROJECT_ROOT / "Release"


def numeric_macro(text, name):
    match = re.search(
        rf"^\s*#define\s+{re.escape(name)}\s+(\d+)\s*$",
        text,
        re.MULTILINE,
    )

    if not match:
        raise RuntimeError(f"{name} is not defined in version.h")

    return match.group(1)


def string_macro(text, name):
    match = re.search(
        rf'^\s*#define\s+{re.escape(name)}\s+"([^"]+)"\s*$',
        text,
        re.MULTILINE,
    )

    if not match:
        raise RuntimeError(f'{name} is not defined as a string in version.h')

    return match.group(1)


def main():
    parser = argparse.ArgumentParser(
        description="Create a versioned firmware release image."
    )
    parser.add_argument(
        "--release",
        action="store_true",
        help="Create a release image. Dirty Git revisions are rejected.",
    )
    args = parser.parse_args()

    if not args.release:
        print("Release image not requested. Skip release image creation.")
        return

    if not VERSION_HEADER.is_file():
        raise FileNotFoundError(
            f"ERROR: version.h file not found: {VERSION_HEADER}"
        )

    if not SOURCE_BIN.is_file():
        raise FileNotFoundError(
            f"ERROR: Source binary file not found: {SOURCE_BIN}"
        )

    version_text = VERSION_HEADER.read_text(encoding="utf-8")

    major = numeric_macro(version_text, "FW_VER_MAJOR")
    minor = numeric_macro(version_text, "FW_VER_MINOR")
    build = numeric_macro(version_text, "FW_VER_BUILD")
    git_revision = string_macro(version_text, "FW_GIT_REV")

    if "-dirty" in git_revision:
        print("ERROR: Git working tree is dirty. Release image will not be created.")
        print(f"Git revision: {git_revision}")
        raise SystemExit(1)

    version = f"v{major}.{minor}.{build}"
    filename = f"{FIRMWARE_FILE_PREFIX}_{version}_{git_revision}.bin"
    destination = RELEASE_DIR / filename

    RELEASE_DIR.mkdir(parents=True, exist_ok=True)
    shutil.copy2(SOURCE_BIN, destination)

    print(f"Created release image: {destination}")


if __name__ == "__main__":
    main()