#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string default_stub() {
  return R"PIK(#!/usr/bin/env bash
set -euo pipefail
self="$0"
marker="__PIKORL_ARCHIVE_BELOW__"
tmpdir="$(mktemp -d)"
cleanup() { rm -rf "$tmpdir"; }
trap cleanup EXIT
line="$(awk "/^${marker}$/ { print NR + 1; exit }" "$self")"
tail -n +"$line" "$self" > "$tmpdir/payload.tar"
tar -xf "$tmpdir/payload.tar" -C "$tmpdir"
entry=""
if [ -f "$tmpdir/entrypoint.sh" ]; then entry="$tmpdir/entrypoint.sh"; fi
if [ -f "$tmpdir/entrypoint.py" ]; then entry="$tmpdir/entrypoint.py"; fi
if [ -z "$entry" ]; then
  entry="$(find "$tmpdir" -maxdepth 2 -type f -name 'entrypoint*' | head -n 1 || true)"
fi
if [ -z "$entry" ]; then
  echo "No entrypoint found in package." >&2
  exit 1
fi
case "$entry" in
  *.py) exec python3 "$entry" ;;
  *) chmod +x "$entry"; exec "$entry" ;;
esac
__PIKORL_ARCHIVE_BELOW__
)PIK";
}

static std::string default_manifest() {
  return R"({
  "features": [
    { "name": "colors",  "path": "colors.lua",  "required": false },
    { "name": "history", "path": "history.lua", "required": false },
    { "name": "macros",  "path": "macros.lua",  "required": false },
    { "name": "syntax",  "path": "syntax.lua",  "required": false },
    { "name": "theme",   "path": "theme.lua",   "required": false },
    { "name": "web",     "path": "web.lua",     "required": false }
  ]
}
)";
}

static std::string default_package_script() {
  return R"PIK(#!/usr/bin/env bash
set -euo pipefail
bundle_dir="$(cd "$(dirname "$0")" && pwd)"
archive_name="${1:-bundle.pikorl}"
stub_file="${PIKORL_STUB:-$bundle_dir/.pikorl_stub.sh}"

if [ ! -f "$stub_file" ]; then
  echo "Missing stub file: $stub_file" >&2
  exit 1
fi

tar_bin="${PIKORL_TAR_BIN:-$bundle_dir/../third_party/libarchive/tar/bsdtar}"
if [ ! -x "$tar_bin" ]; then
  tar_bin="tar"
fi

tmp_tar="$(mktemp)"
trap 'rm -f "$tmp_tar"' EXIT

cd "$bundle_dir"
"$tar_bin" -cf "$tmp_tar" \
  colors.lua history.lua macros.lua MANIFEST.json syntax.lua theme.lua web.lua \
  entrypoint.sh code static
cat "$stub_file" "$tmp_tar" > "$archive_name"
chmod +x "$archive_name"
echo "Created runnable archive: $bundle_dir/$archive_name"
)PIK";
}

static void write_file(const fs::path& path, const std::string& content) {
  std::ofstream out(path, std::ios::binary);
  out << content;
}

int main(int argc, char** argv) {
  std::string stub_path;
  std::vector<std::string> positional;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--stub") {
      if (i + 1 >= argc) {
        std::cerr << "--stub requires a file path\n";
        return 1;
      }
      stub_path = argv[++i];
      continue;
    }
    positional.push_back(arg);
  }

  if (positional.size() != 1) {
    std::cerr << "Usage: pikorl-cli [--stub stub.sh] <bundle-dir>\n";
    return 1;
  }

  fs::path bundle_dir = positional[0];
  if (fs::exists(bundle_dir)) {
    std::cerr << "Target already exists: " << bundle_dir << "\n";
    return 1;
  }

  fs::create_directories(bundle_dir / "code");
  fs::create_directories(bundle_dir / "static");

  write_file(bundle_dir / "colors.lua", "-- colors feature hook\nreturn {}\n");
  write_file(bundle_dir / "history.lua", "-- history feature hook\nreturn {}\n");
  write_file(bundle_dir / "macros.lua", "-- macros feature hook\nreturn {}\n");
  write_file(bundle_dir / "syntax.lua", "-- syntax feature hook\nreturn {}\n");
  write_file(bundle_dir / "theme.lua", "-- theme feature hook\nreturn {}\n");
  write_file(bundle_dir / "web.lua", "-- web feature hook\nreturn {}\n");
  write_file(bundle_dir / "entrypoint.sh", "#!/usr/bin/env bash\necho \"PikoRL bundle entrypoint\"\n");
  write_file(bundle_dir / "MANIFEST.json", default_manifest());
  write_file(bundle_dir / ".pikorl_stub.sh", stub_path.empty() ? default_stub() : "");
  if (!stub_path.empty()) {
    std::ifstream in(stub_path, std::ios::binary);
    if (!in) {
      std::cerr << "Could not read stub: " << stub_path << "\n";
      return 1;
    }
    std::ofstream out(bundle_dir / ".pikorl_stub.sh", std::ios::binary);
    out << in.rdbuf();
  }
  write_file(bundle_dir / "package.sh", default_package_script());

  fs::permissions(bundle_dir / "entrypoint.sh",
                  fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                  fs::perm_options::add);
  fs::permissions(bundle_dir / "package.sh",
                  fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                  fs::perm_options::add);
  fs::permissions(bundle_dir / ".pikorl_stub.sh",
                  fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                  fs::perm_options::add);

  std::cout << "Created bundle scaffold: " << bundle_dir << "\n";
  return 0;
}
