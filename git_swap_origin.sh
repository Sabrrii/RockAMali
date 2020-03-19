#!/bin/bash

#echo "from:"
#grep -e url -e "SWAP" .git/config --color

#echo 'to:'
sed -i 's/#SWAP   url = /#GOTO   url = /;s/        url = /#SWAP   url = /;s/#GOTO   url = /        url = /' .git/config
grep -e url -e "SWAP" .git/config --color
