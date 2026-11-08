# Applications

Applications are executable hosts: Workbench, Player, headless runners,
compilers, and command-line utilities. They consume Core and registered module
APIs; modules must never depend on an application. Every application provides a
headless or no-GPU mode where its use case permits.
