#!/usr/bin/env python
# coding: UTF-8

try:
    import orz
except ImportError:
    import os
    import sys
    current_path = os.path.abspath(os.path.dirname(__file__))
    sys.path.append(os.path.join(current_path))
    import orz

import json
import sys
import os


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: {} filename.sta".format(os.path.split(__file__)[-1]))
        exit(1)

    print(json.dumps(orz.sta2obj(sys.argv[1], binary_mode=2), indent=2))
