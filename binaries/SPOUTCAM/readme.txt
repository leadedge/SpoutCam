SPOUTCAM

SpoutCam is a virtual webcam that is also a Spout receiver.


SPOUTCAM SETTINGS

A separate utility program "SpoutCamSettings" allows options to be selected for SpoutCam.
SpoutCamSettings.exe, 64 bit build, must be copied to this folder,

RESOLUTION

The resolution of SpoutCam depends on :

  o What has been selected by SpoutCamSettings
  o Whether a sender is already running.

1) If a sender is running when SpoutCam starts, it will adapt to the sender resolution if "Active sender" has been selected as the starting resolution (default setting).

2) If any other fixed resolution has been selected, it will always be used whether the sender is started first or not.

3) If no sender is running when SpoutCam starts and "Active sender" has been selected, the resolution is undefined, so the default 640x480 is used.

Some host programs don't accept arbitrary resolutions. If you have trouble, select a fixed resolution so that SpoutCam behaves more like a typical webcam.

The final appearance depends on how the frames are fitted to the aspect ratio of the host webcam display window. They could be fitted to the width, height or stretched to fit, depending on the host.


FRAME RATE

You can use 60fps if it is accepted by the host program and if the CPU performance is sufficient for the processing required. Use 30fps for best compatibility with webcams. The frame rate selected is the “desired” frame rate.  SpoutCam will attempt to meet that frame rate and drop frames if it cannot keep up.

STARTING SENDER

This specifies the sender name that SpoutCam receives from. SpoutCam will only connect to that sender and no other. If the sender is not running at start, SpoutCam displays static and waits until it opens. If the sender closes, SpoutCam waits until it opens again. If the name is left empty, SpoutCam will connect to the first sender running.

OPTIONS

These options may be needed for special applications.

Mirror - mirror image horizontally
Flip   - flip image vertically
Swap   - swap red and blue - RGB<>BRG

REGISTER

If SpoutCam has not been registered, click the 'Register' button. This will register both 32 bit and 64 bit versions of SpoutCam.
After confirmation of success, the button will show 'UnRegister'. Click 'Unregister' to remove SpoutCam from the system.

To update SpoutCam, click 'Unregister' and then 'Register' again. The new copy of Spoutcam will now be ready to use.
				
You can also register SpoutCam manually. There are separate folders for 32 bit and 64 bit versions and you can install both on a 64 bit system. In each folder you will find "_register_run_as_admin.bat". RH click and "Run as Administrator" to register SpoutCam. 

Not all applications support virtual webcams and SpoutCam might not show up for some of them. If you have problems, you can un-register SpoutCam and Register again at any time.

SPOUTCAM PROPERTIES

Settings can also be changed while SpoutCam is running if the host program allows webcam properties to be displayed. The properties dialog provides the same options as SpoutCamSettings with OK, Cancel and Apply buttons. Apply will commit the selected options without closing the dialog. Thereafter, those selections are not reversed by Cancel. For Resolution and Fps changes, SpoutCam has to be re-started by selecting it again and a warning MessageBox is displayed before changes are made. The properties dialog can also be activated manually by "SpoutCamProperties.cmd" for both 32 bit an 64 bit filters. Registration can also be done manually as described above. 

