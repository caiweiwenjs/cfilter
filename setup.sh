#!/bin/bash

#gcc -o my-cups-pdf mybackend.c `cups-config --libs`
make -f pdf_makefile
chmod 0700 my-cups-pdf
chown root my-cups-pdf
chgrp root my-cups-pdf
cp my-cups-pdf /usr/lib/cups/backend/
