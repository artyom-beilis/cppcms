#!/bin/bash

swig -module cppcms_api -php -outdir ../php  -o ../php/cppcms_api.c cppcms.i
