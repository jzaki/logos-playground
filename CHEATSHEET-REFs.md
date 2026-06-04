# Cheatsheet refs

A list of resources for specific tasks...

## Setup build environment

Shown [here](Zero-to-Logos-Program.md#setup)

## Initialize a project from template

`nix flake init -t github:logos-co/logos-module-builder/<version-tag>#<template>`

- `<version-tag>` is optional
- `#<template>` also optional ([full list](https://github.com/logos-co/logos-module-builder/#templates))

## Build and run for dev iterations

- `nix build`
- `nix run`

## Optional: Configure nix c++ project for clangd checks

Prerequisites:

- Tool is installed: `clangd --version`
- Terminal is in the project directory.
- Project (namely dependencies) have been built: `nix build`

For `clangd` to can better understand how the project is built, create the project's `compile_commands.json` with:
`nix develop --command cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

Now create a `.clangd` file with these two paths

```
CompileFlags:
  Add:
    - "-isystem/nix/store/<qtbase hash>-qtbase-6.9.2/include"
    - "-isystem/nix/store/<qtremoteobjects hash>-qtremoteobjects-6.9.2/include"
```

Populating your build hashes from your local `/nix/store`:

- `ls /nix/store/ | grep qtbase`
- `ls /nix/store/ | grep qtremoteobjects`

Finally, ensure logos_sdk.h can be found (...)

> [!TIP]
> If using a vscode flavoured IDE (codium, cursor, ...), the clangd extension will use the above configuration

Also:
`.vscode/settings.json`
{
"clangd.fallbackFlags": ["${env:NIX_CFLAGS_COMPILE}"]
}%

## Build app/module artifacts

`nix build` ([options](https://github.com/logos-co/logos-module-builder/#4-build-your-module))

- Artifacts in `result` dir by default
- Can be an `lgx` file
  - To load into basecamp
- Can be an `install` directory structure (just use lgx)
  - To copy into module or plugin basecamp path
- Can choose `...-portable` varieties ([more on portable](#understanding-portable-variety-of-builds))

## Installing an app/module from lgx file artifact

Install .lgx file via Basecamp UI or CLI tools [here](Zero-to-Logos-App.md#a-load-lgx-via-basecamp-ui)

## Logos Basecamp user directory

Basecamp downloaded from releases will use:

- Linux: `$HOME/.local/share/Logos/LogosBasecamp`
- Mac: `$HOME/Library/Application Support/Logos/LogosBasecamp`

A self-built binary will use:

- Linux: `$HOME/.local/share/Logos/LogosBasecampDev`
- Mac: `$HOME/Library/Application Support/Logos/LogosBasecampDev`

See also [Dev vs Portable](#dev-vs-portable).

Bonus: [Specifying user dir](https://github.com/logos-co/logos-basecamp#parallel-instances---user-dir)

## Dev vs Portable

Most commands/tools default to development, and portable needs to be explicitly stated.

eg `nix build .#lgx` vs `nix build .#lgx-portable`

The default dev commands/tools use dependencies locally in the `/nix/store`, and will install your apps/modules to the Basecamp user dir `.../LogosBasecampDev`.

Portable commands/tools bundle dependencies together (outside of `/nix/store`), and will install your apps/modules to the Basecamp user dir `.../LogosBasecamp`.

## Installing an app/module from `install` directory artifact

**_NB: Easier to just use .lgx described above and installing via cli tools._** Using `install` artifacts requires re-adding write permissions to them that originate from nix store perms.

Copy `install` directory build artifacts directly into LogosBasecamp plugins/modules directories.

Modules and plugins will be installed within `$BASECAMP_DIR/modules` and `$BASECAMP_DIR/plugins` dirs respectively.
Create them if they don't exist: `mkdir $BASECAMP_DIR/modules && mkdir $BASECAMP_DIR/plugins`

- `nix build .#install-portable` # Creates result dir with modules and plugins.
- For modules: `cp -r result/modules/* $BASECAMP_DIR/modules/ && chmod -R u+w $BASECAMP_DIR/modules`
- For plugins: `cp -r result/plugins/* $BASECAMP_DIR/plugins/ && chmod -R u+w $BASECAMP_DIR/plugins`

> [!NOTE] Why aren't there user write permissions?
> The install artifacts were copied from the nix store, which intentionally does not have write permission set
