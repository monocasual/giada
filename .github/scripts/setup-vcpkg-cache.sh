#!/usr/bin/env bash

set -e

# This script configures vcpkg's binary caching, so it can reuse already
# built dependencies instead of rebuilding them from source on every CI run.
# vcpkg's caching works by storing/fetching prebuilt dependency packages
# from a NuGet feed. We use GitHub Packages as that feed, since it's free
# and built into GitHub already.
# The tricky part: vcpkg does NOT use the modern "dotnet nuget" command to
# talk to that feed. It always downloads its own copy of the old "nuget.exe"
# tool and runs that directly. On Windows that's fine, nuget.exe runs
# natively. On Linux/macOS, nuget.exe needs Mono to run at all (that's why
# "mono-complete" was added to linux/install-deps.sh).
# Because of this, we can't register our NuGet feed using "dotnet nuget"
# (as one might normally do), as it would save the feed info in the wrong
# place, one that vcpkg's own nuget.exe never looks at. Instead, we must
# register the feed using that very same nuget.exe binary vcpkg will use
# later. That's what this script does below.
# This script expects GH_TOKEN to already be set in the environment - a
# token with permission to read/write GitHub Packages (e.g. GITHUB_TOKEN).

# The URL of our GitHub Packages NuGet feed, scoped to this repo's owner.
NUGET_SOURCE_URL="https://nuget.pkg.github.com/${GITHUB_REPOSITORY_OWNER}/index.json"

# Ask vcpkg to fetch/locate its own copy of nuget.exe. We reuse this exact
# binary below, instead of any other nuget/dotnet tool that might exist on
# the machine, to make sure we're writing to the config file vcpkg will
# actually read from later.
NUGET_EXE=$(vcpkg fetch nuget | tail -n 1)

# nuget.exe is a Windows tool. On Linux/macOS it has to be run through Mono.
# On Windows, it can just be run directly.
if [[ "$RUNNER_OS" != "Windows" ]]; then
  NUGET_CMD="mono $NUGET_EXE"
else
  NUGET_CMD="$NUGET_EXE"
fi

# Register our GitHub Packages feed as a named NuGet source ("GitHubPackages"),
# with credentials attached, so nuget.exe can authenticate against it later.
$NUGET_CMD sources add \
  -Name "GitHubPackages" \
  -Source "$NUGET_SOURCE_URL" \
  -Username "${GITHUB_REPOSITORY_OWNER}" \
  -Password "${GH_TOKEN}" \
  -StorePasswordInClearText

# Finally, tell vcpkg to actually use this source for binary caching.
# "readwrite" means:
#  - read:  restore already-built packages when available (cache hit)
#  - write: upload newly built packages, so future runs can reuse them
echo "VCPKG_BINARY_SOURCES=clear;nuget,GitHubPackages,readwrite" >> "$GITHUB_ENV"