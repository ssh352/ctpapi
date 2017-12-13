#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import re
import optparse


def main(argv):
    # path = os.path.dirname(os.path.abspath(__file__))
    # f = open(path + '/' + sys.argv[1], 'r')
    parser = optparse.OptionParser(description=sys.modules[__name__].__doc__)
    parser.add_option('--log-level')
    parser.add_option('--no-mem-management', action='store_true')
    parser.add_option('--enable-runtime-checks', action='store_true')
    parser.add_option('--use-asio', action='store_true')
    parser.add_option('--no-exceptions', action='store_true')

    options, args = parser.parse_args(argv)
    param_set = {
        '@CAF_LOG_LEVEL_INT@': '-1' if options.log_level=='-1' else '0',
        '@CAF_LOG_LEVEL@':'{0}'.format(options.log_level),
        '@CAF_NO_MEM_MANAGEMENT_INT@': '0' if options.no_mem_management else '-1',
        '@CAF_ENABLE_RUNTIME_CHECKS_INT@': '0' if options.enable_runtime_checks else '-1',
        '@CAF_USE_ASIO_INT@': '0' if options.use_asio else '-1',
        '@CAF_NO_EXCEPTIONS_INT@':'0' if options.no_exceptions else '-1'
    }

    in_f = open(sys.argv[1], 'r')
    out_f = open(sys.argv[2], 'w')
    lines = in_f.readlines()
    for line in lines:
        for key, val in param_set.iteritems():
            line = line.replace(key, val)
        out_f.write(line)
    in_f.close()
    out_f.close()

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
