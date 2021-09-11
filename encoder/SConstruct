env = Environment()

env.Tool('compilation_db')
env.CompilationDatabase()

Export("env")

SConscript("src/SConscript", variant_dir="build", duplicate=False)

test_env = env.Clone()
Export("test_env")

SConscript("tests/SConscript", variant_dir="build/tests", duplicate=False)
