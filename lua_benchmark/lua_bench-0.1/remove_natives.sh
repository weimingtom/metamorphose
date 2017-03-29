#!/bin/sh
#
FILES=`ls bench/* | grep -v "\.lua" | grep -v "\.txt" `

rm $FILES

