**URL Beamer** is an AUv3 plugin for iOS that launches other apps from within your audio host.

It provides tappable buttons that trigger custom URL schemes – ideal for quick app switching during live performance or studio workflow. You can insert it as either an Audio Effect or an Instrument in hosts like [AUM](https://kymatica.com/apps/aum), then launch external apps with a single tap.
Optionally, each button can also send a MIDI note (on a chosen channel), letting you control other plugins or trigger global functions inside the host.

Features  
• AUv3 plugin formats: Music Effect (aumf), Instrument (aumu)  
• 4 or 8 customizable buttons per instance  
• Buttons launch apps via custom URLs or universal links  
• Optional MIDI Note output (Note + Channel)  
• Basic MIDI Note input for button triggering  
• Minimal layout – fits tightly into your setup  
• File Manager for saving and loading settings  
• AUv3 Plugin State Saving + AU Presets  
• Parameter Mapping for remote MIDI control  
• Designed for AUM – works in other AUv3 hosts too  

Use it to switch apps, load presets, or control devices – all from a tiny plugin window.  
Note:  
URL Beamer does not play sound or process audio. It is a utility plugin intended for workflow enhancements in iOS audio environments.

## Requirements
- iOS device running a host that supports AUv3 plugins (e.g., AUM)
- iOS 15+

## Build Instructions
This plugin was developed using the [JUCE framework](https://juce.com), under the **GNU General Public License (GPLv3)**.

To build:  
1 – Download and install the JUCE framework.  
2 – Clone this repository.  
3 – Open `URLBeamer.jucer` in **Projucer**  
4 – Select your target platform (iOS) and exporter (Xcode).  
5 – Build & run.  
However, to run it on an iOS device, you will need to create your own Certificates and Profiles.  
Alternatively, you could join my Development Team — I would appreciate it.

JUCE is **not included** in this repository. You must download it separately.

## License
This project is licensed under the **GPL v3**.  
© Josef Novotny, 2025

JUCE is © Raw Material Software and available under the GNU GPL v3 or a commercial license.  
The App Icon is © [WPZOOM](https://www.wpzoom.com), licensed under CC BY-SA 3.0  
Source: "satellite 2" on [Icons DB - free custom icons](https://www.iconsdb.com) 

## Contact & Support
For more information, visit [URL Beamer – Support & Info](https://novotny.klingt.org/Apps/URLBeamer/Support)
