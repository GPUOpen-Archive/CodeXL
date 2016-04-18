
////////////////////////////////////////////
//                 CodeXL                 //
////////////////////////////////////////////

This version supports Windows 7 64-bit, Windows 8 64-bit and Linux x86_64 architectures. 

Download
========
CodeXL can be downloaded from:
http://developer.amd.com/tools-and-sdks/opencl-zone/codexl/

Install
=======
On Windows:
To install CodeXL, use the provided MSI file.

On Linux:
To install CodeXL, use the provided RPM file, Debian file, or simply extract the compressed archive onto your hard drive.

For further installation instructions see the CodeXL Quick Start Guide here on the CodeXL web page http://developer.amd.com/tools-and-sdks/opencl-zone/codexl/

Run
===
On Windows:
Use the Start menu and select All Programs -> AMD Developer Tools -> AMD CodeXL -> AMD CodeXL, or if you chose to enable the desktop shortcut installer option during installtion, double click the AMD CodeXL icon on your desktop.

On Linux:
To run CodeXL, launch the CodeXL script from a command shell or double-click it in a file browser window in the installed location.

Updates and support
===================
For support, please visit the AMD CodeXL forum:
http://devgurus.amd.com/community/codexl

The latest version and updates for CodeXL can be found at the AMD Developer Central:
http://developer.amd.com/tools-and-sdks/opencl-zone/codexl/

Open-Source Components
======================
CodeXL uses several open-source components (see Legal section below).
CodeXL uses QT 4.7.4. Source code for QT is available here: http://qt-project.org/downloads
The QT DLLs used by CodeXL are renamed from the original name to include the suffix AmdDt4744. This is done to prevent accidental collision with different versions of QT that may be installed by other 3rd party software packages. The QT source code has not been tempered with and the built binaries are identical to what any user that downloads the source code from the web and builds them will produce, with the exception of the DLL names.

Legal Information
=================

LGPL:
    (Copyright (C) 1991, 1999 Free Software Foundation, Inc.  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA)  Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed. Use of the Qt library is governed by the GNU Lesser General Public License version 2.1 (LGPL v 2.1).

jqPlot:
    copyright © 2009-2011 Chris Leonello

Boost:
    Copyright Beman Dawes, 2003.

TinyXML:
    TinyXML is released under the zlib license
    Files: *
    Copyright: 2000-2007, Lee Thomason 
               2002-2004, Yves Berquin 

    Files: tinystr.*
    Copyright: 2000-2007, Lee Thomason 
               2002-2004, Yves Berquin 
               2005, Tyge Lovset

QScintilla:
    Copyright 2005 by Riverbank Computing Limited <info@riverbankcomputing.co.uk>

LibDwarf:
    Copyright (c) 2007 John Birrell (jb@freebsd.org),  Copyright (c) 2010 Kai Wang,  All rights reserved.  

    1)    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

    2)    THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS “AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Microsoft Detours (CodeXL Windows version only):
    © 2012 Microsoft Corporation
    
OpenCL:
    Copyright (c) 2008-2014 The Khronos Group Inc.

Sincerely,

The CodeXL team
Advanced Micro Devices, Inc.
http://developer.amd.com
