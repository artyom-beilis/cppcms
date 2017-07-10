#!/bin/bash

export LD_LIBRARY_PATH=/opt/cppcms/lib/ 
export PYTHONPATH=..
python manage.py runserver 8000 
