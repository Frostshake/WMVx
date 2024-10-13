# WMVx

A fork of Wow Model Viewer to support legacy and modern client versions, improvements including:

- Updated / modern code base (still using legacy style opengl though)
- Simplified build setup
- 64-bit Build
- Multiple wow client version support (legacy, classic & retail)
- Improved reliablity / stability

Original source from:
- https://code.google.com/archive/p/wowmodelviewer/
- https://bitbucket.org/wowmodelviewer/wowmodelviewer/src/master/
- https://wowdev.wiki/Main_Page

## Quick Start for Users
General usage & troubleshooting guidance can be found in [docs/users.md](/Docs/users.md)

## Technical Notes for Developers
Information on project setup, building and more can be found in [docs/technical.md](/Docs/technical.md)

----

![Screenshot](/Docs/img/screenshot.png "Screenshot")

![Screenshot 2](/Docs/img/screenshot_dark.png "Screenshot 2")

## Expansion Support
- Vanilla (1.12.1)
- TBC (2.4.3)
- WOLTK (3.3.5)
- BFA (8.3.7)
- SL (9.x)
- DF (10.x)
- TWW (11.x)
- Classic and Classic Era (2019+) can be loaded using the equivalent retail profile. 

## Application Feature Support

| Feature | Status |
|-------- |------- |
| __Image Export__ | Basic |
| __3D Export__ | FBX |
| __Settings__ | Basic |
| __Client Detection__ | Yes |

## TODO / Known issues

### Application
- Vanilla & WOTLK to load items based on CSV export from vmangos / trinitycore - (done but need to remove duplicates from CSV file)
- Vanilla & WOTLK to load npcs based on CSV export from vmangos / trinitycore - (currently using old WMV file)
- Character
    - mount / unmount
    - character hands not closing when attaching weapons
- image export doesnt support render to texture
- Implement openglsettings in settings dialog
- NPC's not holding weapons (not possible with just client data?)
- Tidy code
    - tidy all old WMV Code
    - remove c-style code
- Interpolation types not implemented / tested.

### Expansion - Vanilla
- not all animations appear in list, appear to be missing variations
- particles / ribbons not implemented yet
- texture animations not implemented yet

### Expansion - WOTLK
- texture animations dont appear to work / show
- texture transparencies dont appear to be correct - e.g boar
 
### Expansion - BFA
- character tabards - additional tiers not implemented yet.

### Expansion - DF/TWW
- Feature partity with BFA

## Future Ideas
- chaining animations / timed sequences
- animate camera
- items filterable by sub-type, e.g cloth, leather, sword, axe
- lighting
- Spell effects
- npcs, items, models, filterable by expansion
- ability to load individual items
- video exporters
- replace opengl with higher level engine, e.g ogre3d.
