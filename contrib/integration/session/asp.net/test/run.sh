#!/bin/bash
rm -f bin/cppcms.dll
cp ../cppcms.dll bin/
xsp --port 8000 --root ./
