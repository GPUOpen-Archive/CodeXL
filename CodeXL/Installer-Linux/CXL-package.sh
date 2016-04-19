#!/bin/bash

# NOTE: Run this script at the source root. 

VERSION=$1
PACKAGE=CodeXL-${VERSION}

print_usage()
{
	echo "Usage: $0 <version number>"
	exit 1
}


[ $# != 1 ] && print_usage


rm -rf /tmp/${PACKAGE}

mkdir /tmp/${PACKAGE}
cp -r ../../CodeXL /tmp/${PACKAGE}/
tar -C /tmp -czf ${PACKAGE}.tar.gz ${PACKAGE}
rm -rf /tmp/${PACKAGE}
