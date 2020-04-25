#!/bin/bash

# quit on any error
set -e

arch="linux_amd64"
cp ../build/librp_soundboard_${arch}.so plugins/

# extract build number from package.ini
build=$(perl -ne 'print "$1\n" if /^Version = \d+\.\d+\.\d+\.(\d+)/' package.ini)
[ -z $build ] && echo 'invalid build number' && exit 1

read -p "Found build number: ${build}, continue? [Y/n]" yn
case $yn in
    "") ;;
    [Yy]*) break;;
    *) exit 1;;
esac

outfile="releases/${build}/rp_soundboard_${arch}_${build}.ts3_addon"
mkdir -p releases/${build}

# Inject proper platform into copied package.ini file
tmpfile=$(mktemp package.ini.XXXXXX)
cp package.ini ${tmpfile}
sed -i -- "s/^Platforms = win32, win64.*\$/Platforms = ${arch}/g" ${tmpfile}

zip -FS -r -9 ${outfile} ${tmpfile} plugins

#rename temporary file to package.ini
printf "@ $(basename $tmpfile)\n@=package.ini\n"
printf "@ $(basename $tmpfile)\n@=package.ini\n" | zipnote -w ${outfile}

rm ${tmpfile}
