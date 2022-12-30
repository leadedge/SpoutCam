# SpoutCam

SpoutCam is a virtual webcam that is also a Spout receiver.

This project extends the original SpoutCam project and is based on the considerable work and expertise of [Valentin Schmidt](https://github.com/59de44955ebd/SpoutCam) who assembled a self-contained solution for Visual Studio 2017, including the Directshow base classes.

### DirectX version

The Master branch contains a version of SpoutCam based on DirectX. The project requires some but not all of the files from the Spout SDK, currently the Beta branch for Version 2.007. Not all files are needed because the project has been revised to use DirectX 11 instead of OpenGL. The SpoutDX class manages these functions. The files required are included in this repository and can be updated from the [beta branch](https://github.com/leadedge/Spout2/tree/beta/SPOUTSDK/SpoutSDK) if necessary.

### OpenGL version

The OpenGL branch contains the original version of SpoutCam based on OpenGL. The OpenGL version may not be updated after 2.007 release.

### Binaries folder

Binaries for both release and debug compile, are copied as either SpoutCam32.ax or SpoutCam64.ax to the "release" folder and also to a "binaries\SPOUTCAM" folder which contains separate folders to 32bit (SpoutCam32) and 64bit (SpoutCam64) builds

### SpoutCam options

"SpoutCamSettings.exe" has also been copied to the "binaries\SPOUTCAM" folder and can be used to set options for SpoutCam. Documentation for the functions can be found in the program itself using the |?| buttons.

### Register SpoutCam.

"SpoutCamSettings" can also be used to register SpoutCam. Registration can also be done manually. The "SpoutCam32" and "SpoutCam64" folders contain batch files for this purpose. Refer to the readme.txt file in the "binaries\SPOUTCAM" folder. 

### Credit

Original capture source filter authored by Vivek, downloaded from [The March Hare](http://tmhare.mvps.org/downloads/vcam.zip).  
Alterations for Skype compatibility by [John MacCormick](https://github.com/johnmaccormick/MultiCam).  
VS2017 Project by [Valentin Schmidt](https://github.com/59de44955ebd). This project and the 64 bit version of SpoutCam would not have been possible without his work.
