# logos-playground

Misc logos things, hopefully helpful.

## Table of Contents

- [Logos Basecamp](#logos-basecamp)
- [First build](#first-build)
- [Finding modules and corresponding repo urls](#finding-modules-and-corresponding-repo-urls)
- [Logos module in rust](#logos-module-in-rust)

## Logos Basecamp

https://press.logos.co/article/logos-basecamp

Basecamp [0.1.2](https://github.com/logos-co/logos-basecamp/releases?q=0.1.2) is on it's way! Try the Release Candidate.

## First build

Get the tools, build your first local app on Logos.
[Zero to Logos App](Zero-to-Logos-App.md)

## Finding modules and corresponding repo urls

As we approach the 0.1.2 release (currently on RC2), the module lists can be inspected in two ways:

- "latest" or a release tag - showing all modules for master or that release
- pre-release - showing the modules that were ready when the pre-release was made (likely not all modules)

NOTE: this will become simpler when the 0.1.2 is released.

In these methods, you'll noteice most URLs can be manually searched for in [logos-co](https://github.com/logos-co), or [logos-blockchain](https://github.com/logos-blockchain) for some Blockchain and LEZ specific modules.

Note: there is a wallet ui and wallet module in both logos-co, and logos-blockchain.

### Full names and description most urls

See/run `./scripts/list-modules.sh` in this repo (lists latest by default)
Can list pre-releases eg `RELEASE_TAG="build-20260430-d41c2d0-87" ./scripts/list-modules.sh`

### Full descriptions (truncated names, no urls)

From Basecamp Package Manager, (icons on the left, upper-most cube).
Choose release tag from menu in top-right of package manager.

### Full names (truncated descriptions, no urls)

Use the package downloader tool:

```
# Build lgpd
nix build 'github:logos-co/logos-package-downloader#cli' --out-link ./downloader

# List latest available packages
./downloader/bin/lgpd list

# List packages from release
./downloader/bin/lgpd releases
./downloader/bin/lgpd list --release build-20260430-d41c2d0-87
```

(Ref: [Logos Developer Guide](https://github.com/logos-co/logos-tutorial/blob/master/logos-developer-guide.md#53-downloading-and-installing-from-a-registry))

## Logos module in rust

Logos Rust example module - https://github.com/logos-co/logos-rust-example-module

Uses Logos Rust SDK - https://github.com/logos-co/logos-rust-sdk
