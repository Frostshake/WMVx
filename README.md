# WMVx

A fork of Wow Model Viewer to support legacy and modern client versions, improvements including:

- Updated / modern code base (still using legacy style opengl though)
- Simplified build setup
- 64-bit Build
- Multiple wow client version support
- Improved reliablity / stability

Original source from:
- https://code.google.com/archive/p/wowmodelviewer/
- https://bitbucket.org/wowmodelviewer/wowmodelviewer/src/master/
- https://wowdev.wiki/Main_Page

WMVx is still in development, and is in a preview/experimental state - many features have been implemented, however many are still in progress. Known issues / feature status is described below.

![Screenshot](/screenshot.png "Screenshot")

![Screenshot 2](/screenshot_dark.png "Screenshot 2")

## Technical Info

Information on project setup, building and more can be found in technical.md 

## Models / Expansion Support
| Feature        | Vanilla (1.12) | WOTLK (3.3.5) | BFA (8.3.7) | DF (10.2.0) |
|--------------- |-------- |-------- |-------- |--------------|
| __Characters__ | Yes | Yes| Basic | Experimental |
| __Creatures__  | Yes | Yes | Yes | Experimental |
| __Spells__     | Future* | Future* | Future* | Future* |

## Application Feature Support

| Feature | Status |
|-------- |------- |
| __Image Export__ | Basic |
| __3D Export__ | FBX |
| __Settings__ | Not started |
| __Client Detection__ | Basic |

## TODO / Known issues

### Code Quality & Correctness
- correct/tidy std::vector<unique_ptr> reinterperate_cast's
- simplify precompiled headers (nested folder issues)
- resolve TODO's :)

### Application
- Vanilla & WOTLK to load items based on CSV export from vmangos / trinitycore - (done but need to remove duplicates from CSV file)
- Vanilla & WOTLK to load npcs based on CSV export from vmangos / trinitycore - (currently using old WMV file)
- Character
    - mount / unmount
    - character hands not closing when attaching weapons
    - black eye lids on characters, humans have black eyes centers - strangely, toggling a 17xx geoset seems to solve the issue.
- image export doesnt support render to texture
- Implement settings dialog
    - rendering settings
- NPC's not holding weapons (not possible with just client data?)
- potential issues with co-ordinate system, xyz may be in the incorrect order in some places, see camera.
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
- sparse db reading still needs improving, specifically issues with records being of unexpected sizes.
- character tabards - additional tiers not implemented yet.

### Expansion - DF
- Experimental / Feature partity with BFA

### Models - All
- Lights structure
- cameras structure
- events structure

### Incomplete / has blocking issues
- Saving / Loading Scene - requires models to be working fully.

## Future Ideas
- Multi-language client support / Support locales other than enUS
- support user defined files to override with (e.g custom textures)
- chaining animations / timed sequences
- animate camera
- option for normal camera or arc/ball camera, customise controls
- items filterable by sub-type, e.g cloth, leather, sword, axe
- lighting
- Spell effects
- npcs, items, models, filterable by expansion
- ability to load individual items
- video exporters
- ablity to duplicate a model
- open wowhead links
- replace opengl with higher level engine, e.g ogre3d.
- more robust client / locale detection - give user feedback if directory isnt supported version
