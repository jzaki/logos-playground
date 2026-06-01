# Zero to Logos App - a consolidated fly-over

This document is intended to get you right into understanding the essentials of building on Logos.

From Zero to Logos App in the shortest amount of time, with optional extras along the way.

> [!NOTE]
> A "Logos App" runs locally, whereas a "Logos Program" runs on the Logos Execution Zone (execution layer on top of the Logos Blockchain).

# Concept Overview (optional)

## nix

- What: Logos uses nix
- Why: nix builds deterministically for you
- What do: install nix (multi-user mode is the secure way)

## nix flakes

- What: Logos uses nix with flakes (an experimental feature)
- Why: flakes makes modular use of nix easier
- What do: configure to enable flakes

## Hello Logos

- What: Logos uses a Logos App tempate
- Why: Showcases example Logos App as QML UI + C++ backend
- What do: init and run the template (standalone mode)

## Basecamp

- What: Logos develops the platform - Logos Basecamp
- Why: A secure platform to install and run apps as "Plugins", and dependencies as "Core" modules
- What do: Install Basecamp, import and run the example app

# Setup

## tl;dr actions

1. Install nix in multi-user mode (Linux, Mac, Windows WSL2)
   - https://nixos.org/download/
2. Enable flakes in nix config file
   - `mkdir -p ~/.config/nix`
   - `echo 'experimental-features = nix-command flakes' >> ~/.config/nix/nix.conf`

**Note**: You may need to install build tools for c++ make etc if you don't have them already. Eg for debian: `sudo apt install build-essential cmake`

## Setup Details (optional)

**nix**

- Purely functional package manager [ref](https://nix.dev/manual/nix/2.18/introduction#introduction)
- Used to import+build other apps/modules
- **Install https://nixos.org/download/, multi-user mode**

**nix flake**

- The unit for packaging Nix code in a reproducible and discoverable way [ref](https://nix.dev/manual/nix/2.18/command-ref/new-cli/nix3-flake#description)
- **Experimental, needs to be [enabled](https://github.com/logos-co/logos-tutorial/blob/master/tutorial-wrapping-c-library.md#prerequisites)**

**QML (Qt Meta-object Language)**

- Qt is The cross-platform application development framework used in Logos apps/modules
- QML is a user interface markup language (.qml file type)
- Command line tools available from linux package to iterate UI tweaks (Qt Creator sign-up not essential)

**Logos App structure**

- QML UI + C++ backend = UI is decoupled from module interactions
- QML only (not recommended) = slow/sync locking when interfacing with modules

# Hello Logos - standalone

> [!info] If using Codium (or other VSCode flavour)
> Use qt group and nix extensions for syntax highlighting etc.

## Logos module builder

A shared Nix flake library that provides reusable functions for building Logos modules, and can initialise a minimal example app.

The example app is the recommended design pattern of a QML UI + C++ backend. Additional app functionality can be added to the C++ backend (non-reusable), or better still as modules that can be reused.

## Init your app

```
mkdir hello-logos && cd hello-logos
nix flake init -t github:logos-co/logos-module-builder#ui-qml-backend
nix flake update # generate flake.lock file
git init && git add --all # for nix to see files to build
```

If you want to use an tagged version: `nix flake init -t github:logos-co/logos-module-builder/<version-tag>#ui-qml-backend`, the version will need to correspond with a compatible version of Basecamp (next section).

## Build and Run

`nix build` will begin building. The first time is longest since it downloads and builds dependencies, these are cached for next time.

Now run your app in standalone mode:
`nix run` (which will also rebuild any changes)

> [!tip] Some nix tips
>
> - The flake.nix file includes functionality input from the logos-module-builder
> - Run `nix flake update` to update the flake.lock file (eg when changing flake.nix, metadata, cmake files)
> - Remember to `git add` new files for nix to see them

# Create portable artifact, and run in Basecamp

- Download a recent version of Basecamp
  - v0.1.2 Release Candidate 3 [here](https://github.com/logos-co/logos-basecamp/releases/tag/untagged-b54c4d0ae59852515fb2)
  - Or an older [tagged version](https://github.com/logos-co/logos-basecamp/releases?q=prerelease%3Afalse&expanded=true)

> [!tip] If reinstalling from an older version...
> You may need to delete the Basecamp directory previously created: `$HOME/.local/share/Logos/LogosBasecamp`

## Build app into LGX package

To build the app as portable LGX package, run:
`nix build '.#lgx-portable' --out-link result-lgx-portable`

## A: Load LGX via Basecamp UI

- Open the package manager (Lower-left cube icon), and click "Install LGX Package" (top-right green button).
- Select your app .lgx from: `hello-logos/result-lgx-portable/logos-ui_example-module.lgx`
- Then click "Install" (or "Upgrade" if reinstalling).

Now run your application by clicking "Load Plugin" next to it. This will work for anyone with Basecamp (of a compatible version).

## B: Load via CLI

Loading an app via Basecamp is great for sharing with others, but a little tedius for development.
You can use the package manager cli (for portable builds), to do the same thing faster.

First build the package manage cli that creates (portable) .lgx files of apps.

- `nix build 'github:logos-co/logos-package-manager#cli-portable' --out-link ./pm-port`
  - Or, as before, a tagged version - `nix build 'github:logos-co/logos-package-manager/<version-tag>#cli-portable' --out-link ./pm-port-tag`

For convenience set a variable to the Basecamp data directory: `BASECAMP_DIR="$HOME/.local/share/Logos/LogosBasecamp"`

Now to install your app into the plugins dir using Logos Package Manager (portable):
`./pm-port/bin/lgpm --ui-plugins-dir $BASECAMP_DIR/plugins install --file result-lgx-portable/*.lgx`

From Basecamp, go to the package manager and click reload if it does not yet show.

## Optional: Installing other modules

Can select and install from the registry of existing apps (and dependent modules) seen in the Package manager.
If something doesn't install, check that the "Release:" dropdown (top right) corresponds to your version. This could be `latest`, or a pre-release/other tagged version.

# Fast UI dev iterations

Run with [no runtime](https://github.com/logos-co/logos-tutorial/blob/master/tutorial-qml-ui-app.md#76-testing-without-any-runtime)

# Where to next?

- Focus your AI on the QML and C++ files to add to the example
- Wrap a C++ library in a Core module - [Tutorial 1](https://github.com/logos-co/logos-tutorial/blob/master/tutorial-cpp-ui-app.md) (latest or version tag)
- Build an app that interacts with a Core module - [Tutorial 3](https://github.com/logos-co/logos-tutorial/blob/master/tutorial-cpp-ui-app.md) (latest or version tag)
- Wrap a library of your choice (eg hashing function) in a Core module, and update your app to interact with it.
