#!/bin/sh
msgfmt --endianness=big he/LC_MESSAGES/simple.po -o he/LC_MESSAGES/simple.mo
msgfmt he/LC_MESSAGES/default.po -o he/LC_MESSAGES/default.mo
msgfmt he/LC_MESSAGES/fall.po -o he/LC_MESSAGES/fall.mo
msgfmt he_IL/LC_MESSAGES/full.po -o he_IL/LC_MESSAGES/full.mo

