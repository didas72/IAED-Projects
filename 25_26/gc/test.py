import sys
from pycparser import parse_file, c_ast, c_generator


class Scope:
    def __init__(self, parent=None):
        self.parent = parent
        self.renames = {}

    def add(self, old, new):
        self.renames[old] = new

    def lookup(self, name):
        scope = self
        while scope:
            if name in scope.renames:
                return scope.renames[name]
            scope = scope.parent
        return None


class TrkVisitor(c_ast.NodeVisitor):
    def __init__(self):
        self.current_func = None
        self.scope = None

    def push_scope(self):
        self.scope = Scope(self.scope)

    def pop_scope(self):
        self.scope = self.scope.parent

    def visit_FuncDef(self, node):
        self.current_func = node.decl.name
        print(f"\nFunction: {self.current_func}")

        self.push_scope()
        self.visit(node.body)
        self.pop_scope()

    def visit_Compound(self, node):
        # New block scope
        self.push_scope()

        for stmt in node.block_items or []:
            self.visit(stmt)

        self.pop_scope()

    def visit_Decl(self, node):
        if isinstance(node.type, c_ast.TypeDecl):
            type_node = node.type.type

            if isinstance(type_node, c_ast.IdentifierType):
                type_name = " ".join(type_node.names)

                if type_name.startswith("trk_"):
                    old_name = node.name
                    new_name = f"trk_{old_name}"

                    print(f"  Found trk variable: {old_name} (type: {type_name})")

                    # Register in current scope
                    self.scope.add(old_name, new_name)

                    # Rename declaration
                    node.name = new_name

    def visit_ID(self, node):
        new_name = self.scope.lookup(node.name)
        if new_name:
            node.name = new_name


def process_file(filename):
    ast = parse_file(filename, use_cpp=True)

    visitor = TrkVisitor()
    visitor.visit(ast)

    generator = c_generator.CGenerator()
    new_code = generator.visit(ast)

    out_file = filename + ".processed"
    with open(out_file, "w") as f:
        f.write(new_code)

    print(f"\nProcessed file written to: {out_file}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <file.c>")
        sys.exit(1)

    process_file(sys.argv[1])
