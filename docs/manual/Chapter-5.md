Chapter 5 - Example Bundles and Manifest
=========================================

Supported example shapes:

- ``examples/PythonRL``;
- ``examples/RubyRL``.

Manifest rules:

- read ``MANIFEST.json`` via SerdeTk;
- treat manifest entries as authoritative;
- allow partial feature bundles;
- skip missing optional files;
- fail only on missing required resources.

Lua resource loading:

- load Lua files through QaMRpp runtime execution;
- keep feature loading modular.
