
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Pre-build cleanup tool for hispark_taurus_linux_ex.
#
# Symptom:
#   gn gen aborts with:
#     FileExistsError: [Errno 17] File exists: 'toybox' -> 'bin/chmod'
#     (ERROR at //third_party/toybox/BUILD.gn:457)
#
# Cause:
#   third_party/toybox/install.py checks os.path.exists() before re-creating
#   its command symlinks. That call follows symlinks, so dangling links left
#   by a failed/interrupted build are not detected and os.symlink() raises.
#
# Usage (no arguments needed, run from anywhere):
#   python3 vendor/hisilicon/hispark_taurus_linux_ex/prebuild_hispark_linux_ex.py
#
# Only broken symlinks are removed; valid links and binaries are kept.
import os
import shutil
import sys

# Repo root is 3 levels above this script:
#   hispark_taurus_linux_ex -> hisilicon -> vendor -> <root>
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
OUT_DIR = os.path.join(REPO_ROOT, "out", "hispark_taurus", "ipcamera_hispark_taurus_linux_ex")

SRC_INIT_D = os.path.join(
    REPO_ROOT, "vendor", "hisilicon", "hispark_taurus_linux", "init_configs", "etc", "init.d")
DST_ETC_DIR = os.path.join(
    REPO_ROOT, "vendor", "hisilicon", "hispark_taurus_linux_ex", "init_configs", "etc")

LINK_DIRS = ["bin", "sbin", os.path.join("usr", "bin"), os.path.join("usr", "sbin")]


def copy_init_d():
    if not os.path.isdir(SRC_INIT_D):
        print("Warning: %s not found, skip copying" % SRC_INIT_D)
        return 0
    os.makedirs(DST_ETC_DIR, exist_ok=True)
    shutil.copytree(SRC_INIT_D, os.path.join(DST_ETC_DIR, "init.d"), dirs_exist_ok=True)
    print("Copied %s -> %s" % (SRC_INIT_D, DST_ETC_DIR))
    return 0


def main():
    copy_init_d()
    removed = 0
    for sub in LINK_DIRS:
        link_dir = os.path.join(OUT_DIR, sub)
        if not os.path.isdir(link_dir):
            continue
        for name in os.listdir(link_dir):
            path = os.path.join(link_dir, name)
            if os.path.islink(path) and not os.path.exists(path):
                os.remove(path)
                removed += 1
    print("Done, removed %d dangling symlink(s) under %s" % (removed, OUT_DIR))
    return 0


if __name__ == "__main__":
    sys.exit(main())
