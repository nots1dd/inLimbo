#!/bin/bash

doxygen .Doxyfile
cp -r assets/gifs/ docs/
cp index.html docs/
xdg-open docs/index.html
cp -r docs/* new-docs/
