"""Module discovery, validation, and catalog generation."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any


MODULE_ID = re.compile(r"^[a-z][a-z0-9]*(\.[a-z0-9]+)*$")
VERSION = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")
LANGUAGES = {"cpp", "rust"}
DEPENDENCY_KINDS = {"required", "optional"}


@dataclass(frozen=True)
class Module:
    manifest_path: Path
    data: dict[str, Any]

    @property
    def id(self) -> str:
        return self.data["id"]

    @property
    def dependencies(self) -> list[dict[str, str]]:
        return self.data["dependencies"]


def discover_modules(root: Path) -> list[Module]:
    modules = []
    for manifest_path in sorted(root.glob("*/module.json")):
        with manifest_path.open(encoding="utf-8") as manifest_file:
            modules.append(Module(manifest_path, json.load(manifest_file)))
    return modules


def _require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def validate_manifest(module: Module, schema: dict[str, Any] | None = None) -> list[str]:
    data = module.data
    errors: list[str] = []
    schema = schema or {}
    required = set(schema.get("required", [])) or {
        "schemaVersion", "id", "name", "version", "languages", "dependencies", "capabilities"
    }
    properties = set(schema.get("properties", {}))
    if properties:
        unknown = set(data) - properties
        if unknown:
            errors.append(f"{module.manifest_path}: unknown fields: {', '.join(sorted(unknown))}")
    _require(required <= data.keys(), f"{module.manifest_path}: missing required fields", errors)
    if errors:
        return errors
    _require(data["schemaVersion"] == 1, f"{module.id}: schemaVersion must be 1", errors)
    _require(isinstance(data["id"], str) and MODULE_ID.fullmatch(data["id"]) is not None,
             f"{module.id}: invalid module id", errors)
    _require(isinstance(data["name"], str) and bool(data["name"].strip()),
             f"{module.id}: name must not be empty", errors)
    _require(isinstance(data["version"], str) and VERSION.fullmatch(data["version"]) is not None,
             f"{module.id}: version must be semantic x.y.z", errors)
    languages = data["languages"]
    _require(isinstance(languages, list) and bool(languages) and set(languages) <= LANGUAGES,
             f"{module.id}: languages must contain cpp and/or rust", errors)
    dependencies = data["dependencies"]
    _require(isinstance(dependencies, list), f"{module.id}: dependencies must be an array", errors)
    if isinstance(dependencies, list):
        dependency_ids = [dependency.get("id") for dependency in dependencies]
        _require(len(dependency_ids) == len(set(dependency_ids)),
                 f"{module.id}: duplicate dependency", errors)
        for dependency in dependencies:
            _require(
                isinstance(dependency, dict)
                and isinstance(dependency.get("id"), str)
                and dependency.get("kind") in DEPENDENCY_KINDS,
                f"{module.id}: invalid dependency entry",
                errors,
            )
    capabilities = data["capabilities"]
    _require(isinstance(capabilities, list) and len(capabilities) == len(set(capabilities)),
             f"{module.id}: capabilities must be a unique array", errors)
    cmake_target = data.get("cmakeTarget")
    if "cpp" in languages:
        _require(isinstance(cmake_target, str) and bool(cmake_target),
                 f"{module.id}: C++ modules require cmakeTarget", errors)
    return errors


def validate_graph(modules: list[Module]) -> list[str]:
    errors: list[str] = []
    by_id: dict[str, Module] = {}
    for module in modules:
        if module.id in by_id:
            errors.append(f"{module.id}: duplicate module manifest")
        by_id[module.id] = module
    for module in modules:
        for dependency in module.dependencies:
            if dependency["kind"] == "required" and dependency["id"] not in by_id:
                errors.append(f"{module.id}: missing required dependency {dependency['id']}")

    visiting: set[str] = set()
    visited: set[str] = set()

    def visit(module_id: str, path: list[str]) -> None:
        if module_id in visiting:
            cycle = " -> ".join(path[path.index(module_id):] + [module_id])
            errors.append(f"required dependency cycle: {cycle}")
            return
        if module_id in visited or module_id not in by_id:
            return
        visiting.add(module_id)
        for dependency in by_id[module_id].dependencies:
            if dependency["kind"] == "required":
                visit(dependency["id"], path + [dependency["id"]])
        visiting.remove(module_id)
        visited.add(module_id)

    for module in modules:
        visit(module.id, [module.id])
    return errors


def catalog(modules: list[Module]) -> dict[str, Any]:
    return {
        "schemaVersion": 1,
        "modules": [
            {
                "id": module.id,
                "name": module.data["name"],
                "version": module.data["version"],
                "languages": module.data["languages"],
                "dependencies": module.data["dependencies"],
                "capabilities": module.data["capabilities"],
                **({"cmakeTarget": module.data["cmakeTarget"]} if "cmakeTarget" in module.data else {}),
            }
            for module in sorted(modules, key=lambda item: item.id)
        ],
    }
