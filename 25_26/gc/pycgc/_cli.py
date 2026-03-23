import sys
from dataclasses import dataclass
from pathlib import Path

from tree_sitter import Language, Node, Parser
from tree_sitter_c import language


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
        self._edits: list[Edit] = []
        self.code = code

    def apply_edits(self) -> None:
        edits = sorted(self._edits, key=lambda e: e.start, reverse=True)

        for edit in edits:
            self.code = self.code[:edit.start] + edit.content + self.code[edit.end:]

    def add_replace(self, node: Node, content: str) -> None:
        self._edits.append(Edit(node.start_byte, node.end_byte, content))

    def walk(self, node: Node) -> None:
        print(node.text, node.type)

        # 1) Enter scope
        if node.type == "compound_statement":  # { ... }
            print(f"[WARN] Entering scope {node.start_point}")

        # 3) Return statement
        if node.type == "return_statement":
            print(f"[WARN] Return statement {node.start_point}")

        # 2) gc_ variable declarations
        if node.type == "declaration":
            type_node = node.child_by_field_name("type")
            declarator = node.child_by_field_name("declarator")

            if type_node:
                type_text = self.code[type_node.start_byte:type_node.end_byte]

                if type_text.startswith("gc_"):
                    if declarator is None:
                        raise RuntimeError
                    name = self.code[declarator.start_byte:declarator.end_byte] if declarator else "<unknown>"
                    print(f"[WARN] gc_ variable declared: {name} {node.start_point}")
                    self.add_replace(declarator, "gc_"+name)

        # Walk children
        for child in node.children:
            self.walk(child)

        # 4) Exit scope
        if node.type == "compound_statement":
            print(f"[WARN] Leaving scope {node.start_point}")

def analyze_c_code(code: str) -> None:
    parser = Parser()
    parser.language = Language(language())

    code_bytes = bytes(code, "utf8")
    tree = parser.parse(code_bytes)
    root = tree.root_node

    collector = GcCollector(code)
    collector.walk(root)
    print(collector.code)


def main() -> None:
    with Path(sys.argv[1]).open("r") as f:
        code = f.read()

    analyze_c_code(code)
