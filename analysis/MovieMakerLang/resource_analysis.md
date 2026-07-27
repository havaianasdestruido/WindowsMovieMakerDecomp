# MovieMakerLang.dll — Resource String Analysis

## Summary

**MovieMakerLang.dll** is a pure resource-only satellite DLL (212 KB, single `.rsrc` section) containing all English (US) localization strings for Windows Live Movie Maker 2012. It has no code, no exports, no imports — only Win32 resources (RT_STRING tables, RT_DIALOG, RT_MENU, RT_VERSION, PNG icons, DirectUI UIFILE layouts).

**Build**: 16.4.3528.0331 | **Linker**: MSVC 11.00 | **Timestamp**: 2014-04-01

## String Categories

### 1. Application & Window Titles
- `Movie Maker`, `Movie Maker Options`, `Create Custom Setting`, `Add or Remove Effects`
- `Photo Gallery Slide Show`

### 2. Ribbon/Tab Labels
- `Home`, `Animations`, `Visual Effects`, `Project`, `View`, `Format`, `Edit`
- `Video Tools`, `Music Tools`, `Narration Tools`, `Text Tools`, `Captions`
- `Audio and Video`, `Audio and Music`

### 3. Menu & Ribbon Items
- File: `&New project`, `&Open project`, `&Save project`, `Save project &as`, `&Publish movie`, `E&xit`
- Edit: `&Undo`, `&Redo`, `Cu&t`, `&Copy`, `&Paste`, `Select &all`, `&Delete`
- Add: `&Add videos and photos`, `&Add music...`, `&Add sound...`
- Share: `Publish on the web`, `&Manage your services...`, `&Add a plug-in...`
- Credits: `&Director`, `&Starring`, `&Location`, `Sound&track`

### 4. Publishing Targets (Web Services)
| Target | Notes |
|--------|-------|
| `OneDrive` | Default cloud storage |
| `Faceboo&k` | Social sharing |
| `YouT&ube` | Video hosting |
| `Flick&r` | Photo hosting |
| `&Vimeo` | Video hosting |
| `&Blip.TV` | Video hosting |
| `MyS&pace` | Social sharing |
| `DailyMo&tion` | Video hosting |

### 5. Save Movie Profiles
- `Recommended for this pro&ject`
- `For &high-definition display`, `For &computer`, `Burn a &DVD`, `For &email`
- `Windows Phone (lar&ge)`, `&Windows Phone (small)`
- `Apple iPh&one`, `Apple &iPad`
- `Android Phone (&medium)`, `Android Pho&ne (large)`, `Android (72&0p)`, `Android (&1080p)`
- `Windows 7 (7&20p)`, `Windows &8 (1080p)`
- `Zune HD (for &720p display)`, `&Zune HD (for device)`
- `Feature Phone (sma&ll)`, `&Feature Phone (large)`
- `&Audio only`

### 6. Transition Names (Animations Tab)
**Dissolves**: `Bars - horizontal`, `Bars - vertical`, `Crossfade`, `Dissolve`, `Dissolve rough`, `Slide down gap`, `Zoom out`, `Pixelate`
**Artistic**: `Edge detection`, `Posterize`, `None`, `Fade in from white`, `Fade out to black`, `Fade out to white`
**Diagonals**: `Diagonal - box out`, `Diagonal - cross out`, `Diagonal down-right`, `Bow tie - vertical`
**Filled V**: `Filled V down/left/right/up`
**Patterns/Shapes**: `Checkerboard`, `Circle`, `Circles`, `Diamond`, `Eye`, `Heart`, `Keyhole`, `Rectangle`, `Star`, `Stars`, `Wheel`
**Reveals**: `Flip`, `Inset bottom-left/right`, `Inset top-left/right`, `Iris`, `Reveal down/right`, `Roll`, `Shrink in`
**Slide/Spin**: `Slide up`, `Spin`, `Split horizontal`, `Slide down together`
**Shatters**: `Shatter in/right/top-left/top-right`, `Whirlwind/Whirlwind from top`
**Sweeps/Curls**: `Fan in/out/up`, `Page curl bottom-left/right`, `Page curl top-left/right`, `Sweep in/out/up`
**Wipes**: `Wipe down/right - narrow/normal/wide`, `Zig zag horizontal/vertical`
**Cinematic**: `Cinematic - overlay right 2`, `Cinematic - blur`, `Cinematic - fade through gray`, `Cinematic - burst 1/2`, `Cinematic - burst left/right`, `Cinematic - fade left/right`
**Contemporary**: `Contemporary - slide down - narrow/wide 1/2/3`, `Contemporary - drop down 1/2`, `Contemporary - fade 1/2/3`, `Contemporary - fly in left 1/2/3`

### 7. Pan & Zoom Effects
- `None`, `Automatic`, `Pan up/down/left/right`, `Zoom in/out center/top/bottom/left/right`
- `Zoom in center and rotate left/right`, `Pan up along left/right`, `Pan down along left/right`
- `Pan right/left along top/bottom`

### 8. Visual Effects
- `None`, `Blur`, `Black and white`, `Black and white - red/yellow/orange filter`
- `Cyan tone`, `Sepia tone`, `Mirror`, `Mirror horizontal/vertical`
- `3D ripple`, `Fade in from black`, `Hue - cycle entire color spectrum`
- `Pixelate`, `Spin 360`, `Warp`, `Brightness`, `Threshold`
- `Cinematic - overlay left 1/2`, `Cinematic - overlay right 1/2`
- `Video stabilization` (Anti-shake, Anti-shake low/high wobble correction)

### 9. Text Effects
- `Spin in`, `Zoom`, `Zoom in - small/big`, `None`, `Fade`, `Stretch`, `Spin out`
- `Fly in - left/right`, `Scroll`, `Swing down`
- `Cinematic - burst left/right`, `Cinematic - fade left/right`
- `Contemporary - drop down 1/2`, `Contemporary - fade 1/2/3`, `Contemporary - fly in left 1/2/3`

### 10. Error Messages (Selected)
| Error | Context |
|-------|---------|
| `Sorry, Movie Maker had to shut down unexpectedly...` | Crash recovery |
| `Movie Maker doesn't support the selected file type.` | File format |
| `Movie Maker can't open this project file...` | Project corruption |
| `Movie Maker can't find the media file named "%2".` | Missing media |
| `Files can't be added directly from a network folder...` | Network restriction |
| `Movie Maker can't use files that are protected with digital rights management.` | DRM |
| `The required codec for the audio in the file isn't installed...` | Codec missing |
| `The combined file size... is too large.` | Size limit |
| `More than 1,000 photos and videos can't be added...` | Item limit |
| `Sorry, your movie can't be saved. Free some space...` | Disk space |
| `This file might be too large to successfully upload to %s` | Upload limit |
| `Your movie wasn't published successfully...` | Publish failure |
| `Sorry, the service is not currently available.` | Service outage |
| `You don't have permission to save to this location.` | Permissions |
| `Video stabilization failed...` | Stabilization |
| `The video file can't play back and might be corrupted.` | Playback |
| `No webcam was found...` / `No audio device was found...` | Device |
| `Movie Maker is currently saving another movie...` | Concurrency |
| `Project Corruption... has become corrupt.` | Recovery |
| `One or more of the effects files... is corrupted.` | Repair |

### 11. Webcam/Capture/Narration
- `Select an audio device and webcam`
- `Available webcams:`, `Available audio devices:`
- `Start recording video`, `Stop recording video`
- `Start recording narration`, `Stop recording narration`
- `Maximum Resolution:`, `Capture %1!d! (%2!s! %3!s!)`
- `Recording has stopped because the maximum time for recording has been reached.`

### 12. Credit Templates
- `DIRECTED BY` / `[Enter your name here]`
- `STARRING` / `[Enter your cast here]`
- `FILMED ON LOCATION` / `[Enter location here]`
- `SOUNDTRACK` / `[Enter soundtrack info here]`

### 13. Options/Settings Dialogs
- Tabs: `General`, `Advanced`, `Captions`, `Audio and Video`, `Trust Center`
- File preparation: `&Prepare files for faster editing`, `&Prepare files for better stability`
- Customer Experience: `Participate in the Customer Experience Improvement Program`
- `Version 2012 (Build %s)`

### 14. File Type Descriptions
- `Movie Maker Project` (.wlmp), `Movie Maker Video Settings File` (.wlvs)
- `Windows Media Audio File`, `Windows Media Video File`
- `MPEG-4/AAC Audio File`, `MPEG-4/H.264 Video File`

### 15. UI Automation Labels
- `Timeline Pane`, `Storyboard`, `Preview`, `Thumbnail size`
- `Text Track`, `Music Track`, `Visuals Track`, `Audio Track`
- `Progress bar`, `Progress Context`, `Progress Details`, `Statusbar Progress`
- `Playback indicator`, `Preview Scrub Slider`, `Preview Position Label`
- `Waveform group`, `Waveforms`, `Time Zoom`, `Zoom time scale`

### 16. Miscellaneous
- Font: `Segoe UI` (used throughout UI)
- `Windows Live ID`
- Help URL: `http://g.msn.com/5meen_us/122`
- `Portions of this software are based in part on the work of the Independent JPEG Group.`
- Codename reference: `SUNDANCE_RIBBON` (internal codename for Movie Maker was "Sundance")

## Embedded Resource Types
- **PNG images** — Toolbar icons, branding (created with Adobe Photoshop CS5)
- **UIFILE** — DirectUI ribbon/chrome layouts loaded by UXCore.dll
- **RT_DIALOG** — Dialog templates for Options, Custom Settings, Effects management
- **RT_MENU** — Context menus (Cut/Copy/Paste/Delete/Select All)
- **RT_ACCELERATOR** — Keyboard shortcuts (Ctrl+N, Ctrl+S, F11, etc.)
- **RT_VERSION** — VERSIONINFO with build 16.4.3528.0331
