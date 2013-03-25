#!/bin/sh

if [ ! -d "cache" ]; then
mkdir cache
fi


cd cache

echo "Receiving sources.."
if [ ! -d "qmmp-svn" ]; then
svn checkout http://qmmp.googlecode.com/svn/branches/qmmp-0.7/ qmmp-svn
fi

echo "Creating changelog..."
cd qmmp-svn
svn up
svn log > ChangeLog.svn

MAJOR=`cat src/qmmp/qmmp.h | grep "#define QMMP_VERSION_MAJOR" | cut -d " " -f3`
MINOR=`cat src/qmmp/qmmp.h | grep "#define QMMP_VERSION_MINOR" | cut -d " " -f3`
PATCH=`cat src/qmmp/qmmp.h | grep "#define QMMP_VERSION_PATCH" | cut -d " " -f3`

TARBALL=qmmp-$MAJOR.$MINOR.$PATCH


echo Sources name: $TARBALL
cd ..
if [ -d $TARBALL ]; then
echo "Removing previous directory.."
rm -rf $TARBALL
fi

echo "Copying sources.."
cp -r qmmp-svn $TARBALL

cd $TARBALL

echo "Removing svn tags.."
cd utils
./remove_svn_tags.sh
cd ..

echo "Fixing versions.."
sed 's/QMMP_VERSION_STABLE 0/QMMP_VERSION_STABLE 1/' -i src/qmmp/qmmp.h
sed 's/CONFIG += SVN_VERSION/#CONFIG += SVN_VERSION/' -i qmmp.pri
cd ..

echo "Creating tarball.."
tar vcjf ${TARBALL}.tar.bz2 ${TARBALL}/

echo "Removing temporary directory.."
rm -rf ${TARBALL}

echo "Creating md5 sum.."
md5sum -b ${TARBALL}.tar.bz2 > ${TARBALL}.tar.bz2.md5sum

echo "Moving released files.."
cd ..
if [ ! -d "files" ]; then
mkdir files
fi
mv cache/${TARBALL}.tar.bz2 files/
mv cache/${TARBALL}.tar.bz2.md5sum files/

echo ""
echo "****** RELEASED FILES *******"
echo "Tarball: ${TARBALL}.tar.bz2"
echo "MD5: ${TARBALL}.tar.bz2.md5sum"
echo "*****************************"
echo ""
echo "Finished"
