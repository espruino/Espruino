#/bin/bash
# Attempts to upload built binaries to espruino.com

cd `dirname $0`/..

# GITHUB_SHA / TRAVIS_COMMIT
COMMIT=$GITHUB_SHA
# GITHUB_REF_NAME / TRAVIS_BRANCH
BRANCH=$GITHUB_REF_NAME

if [ -z "$COMMIT" ]
then
  echo "NO GITHUB_SHA - not uploading"
  exit
fi
# we don't care if BRANCH is set or not
if [ -z "$UPLOADTOKEN" ]
then
  echo "NO UPLOADTOKEN - not uploading"
  exit
fi

if [[ -n \"$UPLOADTOKEN\" ]]; then 
  ls -d  *.bin *.hex *.tgz *.zip  2> /dev/null | xargs -I {} curl -v -F "binary=@{}" "http://www.espruino.com/travis_upload.php?commit=$COMMIT&branch=$BRANCH&token=$UPLOADTOKEN"; 
fi
