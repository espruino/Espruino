#/bin/bash
# Attempts to upload built binaries to espruino.com


# GITHUB_SHA / TRAVIS_COMMIT
COMMIT=$GITHUB_SHA
# GITHUB_HEAD_REF / TRAVIS_BRANCH
BRANCH=$GITHUB_HEAD_REF

if [ -z "$COMMIT" ]
then
  echo "NO GITHUB_SHA - not uploading"
  exit
fi
if [ -z "$BRANCH" ]
then
  echo "NO GITHUB_HEAD_REF - not uploading"
  exit
fi
if [ -z "$UPLOADTOKEN" ]
then
  echo "NO UPLOADTOKEN - not uploading"
  exit
fi

if [[ -n \"$UPLOADTOKEN\" ]]; then 
  ls *.bin *.hex *.tgz *.zip  2> /dev/null | xargs -I {} curl -v -F \"binary=@{}\" \"http://www.espruino.com/travis_upload.php?commit=$COMMIT&branch=$BRANCH&token=$UPLOADTOKEN\"; 
fi
