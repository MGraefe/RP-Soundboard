# RP-Soundboard-With-Playlist
Easy to use soundboard for Teamspeak 3

A simple yet powerful soundboard that requires no complicated setup! Just install and it's ready to use. No extra tools, no fiddling with push-to-talk settings etc.
It comes with a set of predefined sounds but of course you can choose your own.

# Download and Info
- Official page and downloads: https://www.myteamspeak.com/addons/9e5d66d9-b951-4f46-9b08-0e62909235ee
- Manual downloads: https://github.com/MGraefe/RP-Soundboard/releases

# Features
- Almost any file type (mp3, mp4, wav, flac, ogg, avi, mkv, ...) is supported
- Supports playback of video files (just sound of course, this aint a video player)
- Crop sounds to play only your favorite portion of a sound
- Adjust volume gain for each sound file
- Set keyboard hotkeys for each of your buttons

# Installation
## On Windows
* Simply double-click the downloaded ts3_plugin file and follow the instructions.
* If double-clicking doesn't work, you can also drag&drop the plugin file onto `package_inst.exe` in your Teamspeak installation folder
## On Linux
* Use the `package_inst` executable from your Teamspeak installation. From within the Teamspeak installation folder execute:
  ```bash
  LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./package_inst <path-to-download>
  ```
* alternatively you can open the download with any archive manager (it's just a renamed zip file) and extract the contents of the `plugins` folder to `~/.ts3client/plugins/`
