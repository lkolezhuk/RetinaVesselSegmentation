--------------------------------------------------------------------
Generation and build of an AIA project based on the OpenCV library
--------------------------------------------------------------------
In this tutorial we use Visual Studio 2012, however it can be repla-
ced with any other  development environment / compiler  supported by
CMake (e.g. Eclipse,  XCode, Unix Makefile, etc.),  including  other
O.S. platforms (MacOS and Linux). In the cases, the requirements and
instructions specific for Windows will need to be adapted to the un-
derlying harware/software platform.
--------------------------------------------------------------------
Minimum requirements:

-  x64 architecture
-  Windows 7
-  User Account Control (UAC) disabled (search Google how to do it)
-  Antivirus disabled
-  CMake installed (https://cmake.org/)
-  OpenCV installed (versions 2.*, NO version 3!)
-  Visual Studio 2012 installed and tested
-  AIA template project (should be within this folder)
--------------------------------------------------------------------
Procedure:

1. Launch CMake.

2. In "Where is the source code", insert  the folder  containing the
   source code (this folder)
   *** WARNING ***: the folder path should not contain spaces or sp-
   ecial characters (like 'à', 'è', etc.).
   
3. In "Where to build the binaries", insert the folder where the pro-
   ject files for your platform will be generated. This folder MUST
   be different from the source folder specified at step 2.
   *** WARNING ***: the folder path should not contain spaces or sp-
   ecial characters (like 'à', 'è', etc.).

4. Press "Configure" and select the right generator for your platform.
   In this example, we select "Visual Studio 11 2012 Win64".
   Then, press "Finish".
   
5. CMake will now test the selected compiler / build platform and will
   check for external dependencies (libraries).
   
   If an error regarding the compiler / build platform shows up, it
   means the compiler / build platform does not exist/work properly.
   In this case you need to re-install the build platform or choose
   a different one at step 4.
   
   If an error regarding OpenCV shows up, it's OK (just go on).

6. In "OpenCV_DIR", insert the path of the “build” folder within the
   OpenCV installation folder.

7. Press “Configure”.
   If there are no errors, go on.
   Otherwise, re-check the requirements and repeat steps 1-7.
   
8. Press “Generate” to generate the project files. These will be gen-
   erated within the folder specified at step 3.

9. From now on, the sources (this folder) and the project files (spe-
   cified at step 3 and generated at step 8) will be LINKED FOR EVER.
   Removing/renaming/moving either of these folders/files will result
   in a corruption of the build process. Do not do that!

10.Before we can successfully build and run our projects, we need to
   be sure the OpenCV dynamic libraries (.dll files on Windows, .so on
   Linux, .dylib on MacOS) are visibile to the OS so that they can
   be linked at runtime.
   In Windows, this can be done in Control Panel > System > Advanced
   System Settings > Environment Variables > System Variables, then
   select "PATH", press "Edit", and at the end of the line add the
   separator ";" followed by the full path of the OpenCV folder con-
   taining the dynamic libraries for your building platform.
   In my case, I added:
   ";C:\Program Files\OpenCV-2.4.4\opencv\build\x64\vc11\bin”
   since my architecture is x64 and I am using Visual Studio 2012
   (which corresponds to vc11)
   *** WARNING ***: deleting / corrupting the content of the "PATH"
   envirnoment variable may lead to severe malfunctions of the soft-
   wares installed on your OS. Do this carefully!

11.Go to the destination folder (where the project files have been
   generated) and open the .sln file (solution) with Visual Studio

12.Select the "Release" modality in place of the "Debug" modality from
   the combobox in the top row.
   "Debug" is good for debugging, since it allows line-by-line exec-
   ution and inspection of variables content, but it can slow up the
   execution time (10x-100x slower!!!)

13.Right-click on the project you want to compile (e.g. project0) and
   select "Set as startup project"

14.Build and execute with "Ctrl + F5" or by Build > Start without 
   debugging.
   
   If an error regarding missing DLLs shows up, repeat step 10 and
   reboot Visual Studio.
   
   If the project cannot be executed, perhaps the Antivirus software
   is blocking it (re-check the requirements).
   
   If there are execution errors (app crash or errors displayed on the
   command line), it is likely these are bugs in the source code, which
   means you have completed this tutorial and it's now time to do the
   serious stuff.
--------------------------------------------------------------------