#!/bin/bash
rm -f cppcms.dll
mcs -r:System.Web.dll *.cs -out:cppcms.dll || exit 1

