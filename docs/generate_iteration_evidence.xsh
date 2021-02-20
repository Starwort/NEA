#!/usr/bin/env xonsh
import os
import textwrap

import pyperclip

folder = $(pwd).strip()
cd /tmp
sha = $ARG1
url = f"https://github.com/Starwort/NEA/archive/{sha}.zip"
wget @(url)
unzip @(sha).zip
cd NEA-@(sha)/solver_c
data = ""
template = """- <details><summary><code>{}</code> (click to expand)</summary>

  ```c
  {}
  ```
  </details>
"""
for file in $(ls).split():
    if file in ("debug.c", "debug.h", "test.c", "test.h", "common.c", "common.h", "variadicmacros.h"):
        continue
    data += template.format(file, textwrap.indent(open(file).read(), "  "))
data = "- Code:\n" + textwrap.indent(data, "  ")
pyperclip.copy(data)
cd ../..
rm @(sha).zip
rm -rf NEA-@(sha)
printx(f"{{GREEN}}Sucessfully generated {len(data)} characters of evidence{{RESET}}")
cd @(folder)
