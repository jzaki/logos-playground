#!/usr/bin/env bash
set -euo pipefail

# Optional: set RELEASE_TAG to a specific release tag to list modules for that release instead of latest
# RELEASE_TAG="build-20260430-d41c2d0-87"

CATALOG_BASE_URL="https://github.com/logos-co/logos-modules/releases/"
CATALOG_LATEST="latest/download/list.json"

CATALOG_URL="${CATALOG_BASE_URL}${CATALOG_LATEST}"
if [[ -v RELEASE_TAG && -n "$RELEASE_TAG" ]]; then
    CATALOG_URL="${CATALOG_BASE_URL}download/${RELEASE_TAG}/list.json"
fi

GITMODULES_BASE_URL="https://raw.githubusercontent.com/logos-co/logos-modules/"
GITMODULES_LATEST_PATH="master/.gitmodules"

GITMODULES_URL="${GITMODULES_BASE_URL}${GITMODULES_LATEST_PATH}"
if [[ -v RELEASE_TAG && -n "$RELEASE_TAG" ]]; then
  GITMODULES_URL="${GITMODULES_BASE_URL}/refs/tags/${RELEASE_TAG}/.gitmodules"
fi

VERBOSE=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    -v|--verbose) VERBOSE=1; shift ;;
    *) echo "Unknown option: $1" >&2; exit 1 ;;
  esac
done

echo "Fetching catalog of modules"
echo "Note: urls are from the latest .gitmodules, so may not be complete for pre-releases"

curl -sL "$CATALOG_URL" | python3 -c "
import json, sys, re, urllib.request

data = json.load(sys.stdin)
verbose = $VERBOSE

raw = urllib.request.urlopen('$GITMODULES_URL').read().decode()
module_urls = {}
current = None
for line in raw.splitlines():
    line = line.strip()
    m = re.match(r'\[submodule \"(.+)\"\]', line)
    if m:
        current = m.group(1)
    elif current and line.startswith('url ='):
        url = line.split('=', 1)[1].strip().removesuffix('.git')
        module_urls[current] = url

if not verbose:
    col_w = [8, 12, 20, 45, 0]
    rows = [
        (
            pkg.get('category', ''),
            pkg.get('type', ''),
            pkg.get('name', ''),
            pkg.get('description', ''),
            module_urls.get(pkg.get('name', ''), ''),
        )
        for pkg in data
    ]
    col_w[0] = max(col_w[0], max(len(r[0]) for r in rows))
    col_w[1] = max(col_w[1], max(len(r[1]) for r in rows))
    col_w[2] = max(col_w[2], max(len(r[2]) for r in rows))
    col_w[3] = max(col_w[3], max(len(r[3]) for r in rows))
    header = f\"{'CATEGORY':<{col_w[0]}}  {'TYPE':<{col_w[1]}}  {'NAME':<{col_w[2]}}  {'URL':<{col_w[3]}}  DESCRIPTION\"
    print(header)
    print('-' * len(header))
    for cat, typ, name, url, desc in rows:
        print(f\"{cat:<{col_w[0]}}  {typ:<{col_w[1]}}  {name:<{col_w[2]}}  {url:<{col_w[3]}}  {desc}\")
elif verbose:
    for pkg in data:
        module_name = pkg.get('moduleName', '')
        url = module_urls.get(pkg.get('name', ''), '')
        print(f\"name:         {pkg.get('name', '')} ({url})\")
        print(f\"description:  {pkg.get('description', '')}\")
        print(f\"type:         {pkg.get('type', '')}\")
        print(f\"moduleName:   {module_name}\")
        print(f\"package:      {pkg.get('package', '')}\")
        print(f\"category:     {pkg.get('category', '')}\")
        print(f\"author:       {pkg.get('author', '')}\")
        print(f\"version:      {pkg.get('version', '')}\")
        print(f\"variants:     {', '.join(pkg.get('variants', []))}\")
        deps = pkg.get('dependencies', [])
        print(f\"dependencies: {', '.join(deps) if deps else '(none)'}\")
        print()
"
