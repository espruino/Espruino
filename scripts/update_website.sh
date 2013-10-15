#!/bin/bash
cd `dirname $0`
cd ..
DIR=`pwd`
WEBSITEDIR=$DIR/../espruinowebsite
ESPRUINODOCS=$DIR/../EspruinoDocs
CMSDIR=$DIR/../espruinowebsite/cms
REFERENCEDIR=$DIR/../espruinowebsite/reference
BOARDIMGDIR=$WEBSITEDIR/www/img

echo Updating Board Docs
echo "<h1>Espruino Hardware Reference</h1>" > NewReference.html
echo "<p>The Espruino Software will run on a variety of boards. The Espruino Board, <a href=\"/kick\">currently on KickStarter</a>, has been specially designed to complement our software and is the only board that we actively support. Please click on the thumbnails below to see diagrams of each board with all pins and their capabilities marked</p>" >> NewReference.html
echo "<h2>Espruino Board - Supported</h2>" >> NewReference.html
 BOARDNAME=ESPRUINOBOARD
 NICENAME=`python scripts/get_board_name.py $BOARDNAME`
 echo $BOARDNAME = $NICENAME 
 python scripts/build_board_docs.py ${BOARDNAME}  || { echo 'Build failed' ; exit 1; }
 grep boards/${BOARDNAME}.html -v -f scripts/website_banned_lines.txt > ${REFERENCEDIR}/Reference${BOARDNAME}.html
 cp boards/img/${BOARDNAME}.* ${BOARDIMGDIR}
 convert boards/img/${BOARDNAME}.* -resize 256x256 ${BOARDIMGDIR}/${BOARDNAME}_thumb.jpg
 echo -e "<center><span style=\"text-align:center;margin:10px;width:200px;\"><a href=\"Reference${BOARDNAME}\"><img src=\"img/${BOARDNAME}_thumb.jpg\" alt=\"${NICENAME}\"><br/>${NICENAME}</a></span></center>" >> NewReference.html
echo "<h2>Other Boards - Unsupported</h2>" >> NewReference.html
echo "<div id=\"boards\" style=\"display:inline-block;\">" >> NewReference.html
mkdir $BOARDIMGDIR
for BOARDNAME in STM32VLDISCOVERY STM32F3DISCOVERY STM32F4DISCOVERY OLIMEXINO_STM32 HYSTM32_24 HYSTM32_28 HYSTM32_32
do
 NICENAME=`python scripts/get_board_name.py $BOARDNAME`
 echo $BOARDNAME = $NICENAME 
 python scripts/build_board_docs.py ${BOARDNAME}  || { echo 'Build failed' ; exit 1; }
 grep boards/${BOARDNAME}.html -v -f scripts/website_banned_lines.txt > ${REFERENCEDIR}/Reference${BOARDNAME}.html
 cp boards/img/${BOARDNAME}.* ${BOARDIMGDIR}
 convert boards/img/${BOARDNAME}.* -resize 128x128 ${BOARDIMGDIR}/${BOARDNAME}_thumb.jpg
 echo -e "<span style=\"display:inline-block;text-align:center;margin:10px;width:200px;\"><a href=\"Reference${BOARDNAME}\"><img src=\"img/${BOARDNAME}_thumb.jpg\" alt=\"${NICENAME}\"><br/>${NICENAME}</a></span>" >> NewReference.html
done
echo "</div>"
echo "<p>&nbsp;</p>"

echo Updating Reference.html
python scripts/build_docs.py
grep functions.html -v -f scripts/website_banned_lines.txt >> NewReference.html
rm functions.html
mv NewReference.html ${REFERENCEDIR}/Reference.html

echo Updating ChangeLog.html
sed -n "/FILEBEGIN/,/CHANGELOGBEGIN/p" ${CMSDIR}/ChangeLog.html > tmp.html
bash $DIR/scripts/extract_changelog.sh | sed "s/</\&lt;/g" >> tmp.html
sed -n "/CHANGELOGEND/,/./p" ${CMSDIR}/ChangeLog.html >> tmp.html
mv tmp.html  ${CMSDIR}/ChangeLog.html

cd $DIR/archives
CURRENTZIP=`ls espruino_1v*.zip | sort | tail -1`
echo Current zip = $CURRENTZIP
cp -v $CURRENTZIP $WEBSITEDIR/www/files

CURRENTVERSION=`echo $CURRENTZIP | sed -ne "s/.*\(1v[0-9][0-9]\).*/\1/p"`
echo Current version = $CURRENTVERSION
sed -i "s/1v[0-9][0-9]/$CURRENTVERSION/g" $CMSDIR/Download.html

cd $ESPRUINODOCS
./build.sh
