# SpoutCam

SpoutCam is a virtual webcam that is also a Spout receiver.

This project extends the original SpoutCam project and is based on the considerable work and expertise of [Valentin Schmidt](https://github.com/59de44955ebd/SpoutCam) who assembled a self-contained solution for Visual Studio 2017, including the Directshow base classes.

### DirectX version

The Master branch contains a version of SpoutCam based on DirectX. The project requires some but not all of the files from the Spout SDK, currently the Beta branch for Version 2.007. Not all files are needed because the project has been revised to use DirectX 11 instead of OpenGL. The SpoutDX class manages these functions. The files required are included in this repository and can be updated from the [beta branch](https://github.com/leadedge/Spout2/tree/beta/SPOUTSDK/SpoutSDK) if necessary.

### OpenGL version

The OpenGL branch contains a version of SpoutCam based on OpenGL. The OpenGL version may not be updated after 2.007 release.

### Output folder

Binaries for both release and debug compile, are copied as either SpoutCam32.ax or SpoutCam64.ax to the "release" folder.

### Register SpoutCam.

SpoutCam 32 bit is registered if Spout 2.006 is installed. If you build using this repository. The "release" folder has batch files which can be used to register or unregister different versions of SpoutCam. 

Please also refer to the 2.007 version of [SpoutCamSettings](https://github.com/leadedge/Spout2/tree/beta/SPOUTCAM) which allows resolution and frame rate to be changed.

The program can also register SpoutCam and binaries are included in the repository. To register different ones, copy your binary files to the SpoutCam32 or SpoutCam64 folders before running SpoutCamSettings.

### Credit

Original capture source filter authored by Vivek, downloaded from [The March Hare](http://tmhare.mvps.org/downloads/vcam.zip).  
Alterations for Skype compatibility by [John MacCormick](https://github.com/johnmaccormick/MultiCam).  
VS2017 Project by [Valentin Schmidt](https://github.com/59de44955ebd). This project and the 64 bit version of SpoutCam would not have been possible without his work.
