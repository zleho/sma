#!/bin/sh
rm -f doc.pdf
pandoc -S doc.md -o doc.pdf
