import sys
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

from tree_sitter import Language, Node, Parser
from tree_sitter_c import language

if TYPE_CHECKING:
    from collections.abc import Callable


@dataclass
class Edit:
    start: int
    end: int
    content: str

@dataclass
class Scope:
    parent: "Scope | None"
    gc_vars: set[str]

class GcCollector:
    def __init__(self, code: str) -> None:
        self._node_handler: dict[str, Callable[[Node], None]] = {
            "declaration": self.handle_declaration,
        }
        self._unhandled_types: set[str] = { "compound_statement" }

        self._edits: list[Edit] = []
        self.code = code
        self._scope: Scope | None = Scope(None, set()) # Root scope

    def scope(self) -> Scope:
        if self._scope is None:
            raise ValueError
        return self._scope

    def enter_scope(self, node: Node) -> None:
        print(f"Entering scope {node.start_point}")
        self._scope = Scope(self._scope, set())

    def leave_scope(self, node: Node) -> None:
        print(f"Leaving scope {node.end_point}")
        self._scope = self.scope().parent

    def apply_edits(self) -> None:
        edits = sorted(self._edits, key=lambda e: e.start, reverse=True)

        for edit in edits:
            self.code = self.code[:edit.start] + edit.content + self.code[edit.end:]

    def add_replace(self, node: Node, content: str) -> None:
        self._edits.append(Edit(node.start_byte, node.end_byte, content))

    def add_prefix(self, node: Node, content: str) -> None:
        self._edits.append(Edit(node.start_byte, node.start_byte, content))

    def handle_declaration(self, node: Node) -> None:
        declarators = node.children_by_field_name("declarator")

        for declarator in declarators:
            decl: Node = declarator
            while decl.type != "identifier": # Find identifier in pointers and inits
                lower = decl.child_by_field_name("declarator")
                if lower is None:
                    raise RuntimeError
                decl = lower
            name = self.code[decl.start_byte:decl.end_byte] if decl else "<unknown>"

            # Ignore non gc_ variables
            if not name.startswith("gc_"):
                continue

            print(f"gc_ variable declared: {name} {node.start_point}")
            self.add_prefix(decl, "!")

    def walk(self, node: Node) -> None:
        # Scope enter special case
        if node.type == "compound_statement":
            self.enter_scope(node)

        handler = self._node_handler.get(node.type)
        if handler is None:
            if node.type not in self._unhandled_types:
                #print(f"[WARN] Unhandled node type: {node.type}")
                self._unhandled_types.add(node.type)
        else:
            # Invoke handler
            handler(node)

        # Walk children
        for child in node.children:
            self.walk(child)

        # Scope leave special case
        if node.type == "compound_statement":
            self.leave_scope(node)


def analyze_c_code(code: str) -> None:
    parser = Parser()
    parser.language = Language(language())

    code_bytes = bytes(code, "utf8")
    tree = parser.parse(code_bytes)
    root = tree.root_node

    collector = GcCollector(code)
    collector.walk(root)
    collector.apply_edits()
    print(collector.code)


def main() -> None:
    with Path(sys.argv[1]).open("r") as f:
        code = f.read()

    analyze_c_code(code)
