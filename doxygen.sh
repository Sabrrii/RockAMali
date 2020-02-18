#!/bin/bash

#get version
program=process
program=process_sequential
make $program && ./$program --version | grep 'v.\..\..' | head -n 1 > VERSION

#documentation
##logo
cp -p doc/logo.png doc/html/
##version
VERSION=`cat VERSION | grep 'v.\..\..'`
cat Doxyfile.template | sed 's/##VERSION##/'$VERSION'/' > Doxyfile
##clean *_help.output
for f in *_help.output
do
  ft=`basename $f _help.output`.help.output
  cat $f | sed 's/..0.31.59m//;s/..1m//;s/..0.0.0m//;s/..0.32.59m//;' > $ft
  mv $ft $f
done

##graph
cp -p process_perfs.png doc/store_plot.png

##make (html and prepare latex)
doxygen

ls doc/html/index.html
#GUI notify
notify-send 'documentation for RockAMali '$VERSION' doxygen ends,\ni.e. '$0' from "'$PWD'".' --icon=/usr/share/hardinfo/pixmaps/report-large.png -t 4321

exit

##make latex
cd doc/latex
make

