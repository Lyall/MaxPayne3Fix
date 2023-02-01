# Max Payne 3 Fix
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)</br>
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/MaxPayne3Fix/total.svg)](https://github.com/Lyall/MaxPayne3Fix/releases)

This is a fix for several issues with ultrawide/wider displays in Max Payne 3.

## Features
- Corrects >2.3 aspect ratios causing the game to enter multi-monitor mode.
- Stops fullscreen mode from using your highest-available resolution. (Thanks Rose!)
- Correctly scaled FMV playback.
- Correctly scaled loading screens.
- Spanned or centred 16:9 HUD adjustment.
- Gameplay FOV adjustment without affecting cutscenes/other camera types.

## Installation
- Grab the latest release of MaxPayne3Fix from [here.](https://github.com/Lyall/MaxPayne3Fix/releases)
- Extract the contents of the release zip in to the game directory.<br />(e.g. "**steamapps\common\Max Payne 3\Max Payne 3**" for Steam).

## Configuration
- See **MaxPayne3Fix.ini** to adjust various aspects of the fix.

## Known Issues
Please report any issues you see.

## Screenshots

| ![scaled FMVs](https://user-images.githubusercontent.com/695941/216190486-a78afa0d-827d-48ab-bd7f-53fb0782caac.gif) |
|:--:|
| Correctly scaled FMVs. |

| ![loading screens](https://user-images.githubusercontent.com/695941/216190470-7670bed0-a7a0-4e88-9d5d-59ef239fc657.gif) |
|:--:|
| Correctly scaled loading screens. |

| ![centred hud](https://user-images.githubusercontent.com/695941/216190476-d69b284d-9b0a-4a23-a701-479bee109776.gif) |
|:--:|
| Spanned/16:9 centred HUD. |

## Credits
[Rose](https://github.com/RoseTheFlower) for sharing a solution to stop fullscreen mode from using your highest-available resolution. <br />
[jackfuste's original post](https://www.wsgf.org/phpBB3/viewtopic.php?p=172338#p172338) on the WSGF forums for part of the aspect/resolution fix. <br />
[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading. <br />
[inih](https://github.com/jtilly/inih) for ini reading. <br />
[Loguru](https://github.com/emilk/loguru) for logging. <br />
[length-disassembler](https://github.com/Nomade040/length-disassembler) for length disassembly.
