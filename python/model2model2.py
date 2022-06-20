#!/usr/bin/env python

try:
    import orz
except ImportError:
    import os
    import sys
    current_path = os.path.abspath(os.path.dirname(__file__))
    sys.path.append(os.path.join(current_path))
    import orz

import os
import sys


def timestamp(filename):
    if not os.path.exists(filename):
        return 0
    stat = os.stat(filename)
    return stat.st_mtime


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: {} input_dir [output_dir]".format(os.path.split(__file__)[-1]))
        exit(1)

    input_dir = sys.argv[1]
    output_dir = input_dir
    if len(sys.argv) > 2:
        output_dir = sys.argv[2]

    filenames = os.listdir(input_dir)

    count_keep = 0
    count_modify = 0

    for filename in filenames:
        if filename[-5:] != '.json':
            continue
        name, ext = os.path.splitext(filename)
        filename_sta = name + '.sta'
        input_filename = os.path.join(input_dir, filename)
        output_filename = os.path.join(output_dir, filename_sta)
        if timestamp(output_filename) < timestamp(input_filename):
            print('Converting %s' % input_filename)
            orz.json2sta(input_filename, output_filename)
            count_modify += 1
        else:
            print('Keeping %s' % input_filename)
            count_keep += 1

    count_total = count_modify + count_keep

    print("Total: %d. Modified: %d, kept: %d" % (count_total, count_modify, count_keep))
