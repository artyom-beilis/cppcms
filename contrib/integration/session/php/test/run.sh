#!/bin/bash
php5 -S 0.0.0.0:8000 -d enable_dl=On -d extension=../cppcms_api.so  -t ./ 
