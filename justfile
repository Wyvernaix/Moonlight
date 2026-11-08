set shell := ["sh", "-cu"]

build:
    python python/build.py

build-release:
    python python/build.py --configuration release

native:
    python python/build.py --skip modules --skip rust --skip managed

rust:
    python python/build.py --skip modules --skip native --skip managed

managed:
    python python/build.py --skip modules --skip native --skip rust

validate:
    python python/build.py

workbench:
    python python/build.py --workbench --skip native

bootstrap:
    python python/bootstrap.py

plan:
    python python/build.py --dry-run
