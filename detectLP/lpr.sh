#!/bin/sh
#script
EXPECTED_ARGS=1
E_BADARGS=1
SUCCESS=0

rm -f output/*

if [ $# -ne $EXPECTED_ARGS ]
then
    echo "Usage: ./lpr <input.img>"
    exit $E_BADARGS
fi

./0_locate_lp/locate.out $1
./1_text_isolation/txtiso.out output/contour0.jpg
./2_character_segmentation/vipb.out output/cleanIm.jpg
#./3_ocr/ocr.out

exit $SUCCESS
