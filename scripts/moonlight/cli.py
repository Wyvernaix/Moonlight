"""Command-line entry point for Moonlight module tooling."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .module_tool import catalog, discover_modules, validate_graph, validate_manifest


def main() -> int:
    parser = argparse.ArgumentParser(prog="moonlight", description="Moonlight project tooling")
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="repository root")
    subparsers = parser.add_subparsers(dest="command", required=True)
    for command in ("list", "validate"):
        subparsers.add_parser(command)
    export = subparsers.add_parser("export", help="write the module catalog")
    export.add_argument("--out", type=Path, default=Path("build/graph/module-catalog.json"))
    args = parser.parse_args()
    modules_root = args.root / "modules"
    modules = discover_modules(modules_root)
    schema_candidates = (
        args.root / "data" / "schemas" / "module" / "module-manifest.schema.json",
        args.root / "schemas" / "module-manifest.schema.json",
    )
    schema_path = next((path for path in schema_candidates if path.exists()), schema_candidates[0])
    try:
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"error: cannot load module schema {schema_path}: {error}", file=sys.stderr)
        return 1

    errors = [
        error
        for module in modules
        for error in validate_manifest(module, schema)
    ]
    if not errors:
        errors.extend(validate_graph(modules))
    if args.command == "list":
        for module in modules:
            print(f"{module.id} {module.data['version']} ({', '.join(module.data['languages'])})")
        return 1 if errors else 0
    if errors:
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
        return 1
    if args.command == "validate":
        print(f"validated {len(modules)} module(s)")
        return 0
    output = args.out if args.out.is_absolute() else args.root / args.out
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(catalog(modules), indent=2) + "\n", encoding="utf-8")
    print(f"wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
