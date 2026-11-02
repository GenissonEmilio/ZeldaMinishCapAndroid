#!/usr/bin/env bash
set -euo pipefail
PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
: "${ANDROID_HOME:?Set ANDROID_HOME to an Android SDK with API 35, NDK 28.2.13676358 and CMake 3.31.6}"
BUILD_KIND=${1:-Release}
case "$BUILD_KIND" in Debug|Release) ;; *) echo 'Use Debug or Release' >&2; exit 2;; esac
test -f "$PROJECT_ROOT/generated/dispatch_table.cpp" || { echo 'Run tools/android/prepare.py first' >&2; exit 2; }
test -f "$PROJECT_ROOT/generated/eu/eu_dispatch_table.cpp" || { echo 'Generate Europe too: tools/android/prepare.py --rom <EU.gba>' >&2; exit 2; }
cd "$PROJECT_ROOT/android"
exec ./gradlew --no-daemon ":app:assemble$BUILD_KIND" -PgbaNativeJobs="${MINISH_BUILD_JOBS:-2}"
