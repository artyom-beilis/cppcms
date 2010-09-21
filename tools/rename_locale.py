#!/usr/bin/python

import sys
import re

boost = re.compile('^(\s*namespace\s+)boost([\s\{].*)$')
cstdint = re.compile('^\s*#include\s*<boost/cstdint.hpp>\s*$')
config = re.compile('^\s*#include\s*<boost/locale/config.hpp>\s*$')
lochpp = re.compile('^\s*#include\s*<boost/locale.hpp>\s*$')
anylochpp = re.compile('^\s*#include\s*<boost/locale/(.*).hpp>\s*$')
backinc = re.compile('^\s*#include\s*"../src/(.*).hpp"\s*$')
srchpp = re.compile('^\s*#include\s*"(.*).hpp"\s*$')
 

for line in sys.stdin:
	m=boost.match(line)
	if m:
		print m.group(1) +'cppcms' + m.group(2)
	elif cstdint.match(line):
		print '#include "cstdint.h"'
	elif config.match(line):
		print '#include "defs.h"'
		print '#include "config.h"'
	elif anylochpp.match(line):
		print '#include "locale_%s.h"' % anylochpp.match(line).group(1)
	elif backinc.match(line):
		print '#include "locale_src_%s.hpp"' % backinc.match(line).group(1)
	elif srchpp.match(line):
		print '#include "locale_src_%s.hpp"' % srchpp.match(line).group(1)
	elif lochpp.match(line):
		print '#include "localization.h"'
	else:
		line = line.replace('BOOST_MSVC','_MSC_VER')
		line = line.replace('boost::locale','cppcms::locale')
		line = line.replace('BOOST_VERSION >= 103600','1')
		line = line.replace('BOOST_LOCALE_DECL','CPPCMS_API')
		line = line.replace('BOOST','CPPCMS')
		print line,
		


