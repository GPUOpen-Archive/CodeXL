#!/bin/bash

# This will merge 3 static libraries libCXLBaseTools.a libCXLOSWrappers.a and libCXLPowerProfileAPI_part.a
# into libCXLPowerProfileAPI.a.
makearchive='create variant_release/AMDTPowerProfileAPI/libCXLPowerProfileAPI.a\n addlib variant_release/AMDTBaseTools/libCXLBaseTools.a\n addlib variant_release/AMDTOSWrappers/libCXLOSWrappers.a\n addlib variant_release/AMDTPowerProfileAPI/libCXLPowerProfileAPI_part.a\n  save\n end\n'
echo -e $makearchive | ar -M
