#!/bin/bash
cd `dirname $0`
cd ..
DIR=`pwd`
WEBSITEDIR=$DIR/../espruinowebsite
CMSDIR=$DIR/../espruinowebsite/cms

echo Updating Reference.html
sed -n "/FILEBEGIN/,/FUNCTIONSBEGIN/p" ${CMSDIR}/Reference.html > tmp.html
bash $DIR/scripts/extract_docs.sh | sed "s/</\&lt;/g" >> tmp.html
sed -n "/FUNCTIONSEND/,/./p" ${CMSDIR}/Reference.html >> tmp.html
mv tmp.html  ${CMSDIR}/Reference.html

echo Updating ChangeLog.html
sed -n "/FILEBEGIN/,/CHANGELOGBEGIN/p" ${CMSDIR}/ChangeLog.html > tmp.html
bash $DIR/scripts/extract_changelog.sh | sed "s/</\&lt;/g" >> tmp.html
sed -n "/CHANGELOGEND/,/./p" ${CMSDIR}/ChangeLog.html >> tmp.html
mv tmp.html  ${CMSDIR}/ChangeLog.html
