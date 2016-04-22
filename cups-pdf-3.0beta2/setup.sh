#!/bin/bash

rm -rf cups-pdf
gcc -O9 -s -o cups-pdf ./src/cups-pdf.c -I./src/ -lcups
#gcc -o cups-pdf ./src/cups-pdf.c -I./src/ -lcups
chmod 0700 cups-pdf
chown root cups-pdf
chgrp root cups-pdf
cp cups-pdf /usr/lib/cups/backend/
cp ./extra/cups-pdf.conf /etc/cups
#cp ./extra/CUPS-PDF_noopt.ppd /usr/share/cups/model
