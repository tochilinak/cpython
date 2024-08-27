import runpy
import sys

sys.path = [r'Lib\ensurepip\_bundled\pip-24.0-py3-none-any.whl'] + sys.path

print(sys.path)

runpy.run_module("pip", run_name="__main__", alter_sys=True)
