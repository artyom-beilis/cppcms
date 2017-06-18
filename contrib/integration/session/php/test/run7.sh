#!/bin/bash
php7.0 -S 0.0.0.0:8000 -d enable_dl=On -d extension=../php7/cppcms_api.so  -t ./ 
