VER='2_20'
INCLUDE_TREE = {
        'debug-x86'         : 'Include/',
        'release-x86'       : 'Include/',
        'debug-x86_64'      : 'Include/',
        'release-x86_64'    : 'Include/'
        }
LIBRARY_TREE = {
        'debug-x86'        : 'Bin/Linx86',
        'release-x86'      : 'Bin/Linx86',
        'debug-x86_64'     : 'Bin/Linx64',
        'release-x86_64'   : 'Bin/Linx64',
        }
LIBS = []

COPY_LIBS = ['libGPUPerfAPICounters.so']
