#!/bin/sh

# in version.h find the string "unknown_manual_build and change that to the infor form git"
sed -i -e "s@unknown_manual_build@`git describe --abbrev=4 --dirty --always --tags`@g" ${SRCROOT}/src/version.h


