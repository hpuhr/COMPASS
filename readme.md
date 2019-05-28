## Description
Based on a dynamic database schema, recorded air traffic surveillance data can be inserted, retrieved and inspected.

- Support of multiple database systems, e.g. Sqlite3, MySQL
- Support of multiple, configurable database schemas, e.g. SCDB, Simple
- Dynamic JSON import from SDDL, ADS-B exchange, OpenSky Network
- Dynamic ASTERIX import using [jASTERIX](https://github.com/hpuhr/jASTERIX)
- MySQL database import and management of SCDB databases
- High performance processing, low memory footprint
- Utilization of application during loading procedure
- Views for data inspection
- Simple custom filter generation
- Supported Database Objects
  - Radar plots
  - System Tracks and Reference Trajectories
  - MLAT & WAM target reports
  - ADS-B target reports
- XML-based configuration files
- Multiple coexisting configurations, usage chosen during runtime
- Based on Open Source libraries
- Runs on generic hardware

Please refer to the releases page for the user manual and the AppImage. Please do read the user manual before running the application.

![alt text](https://github.com/hpuhr/ATSDB/blob/master/doc/screenshots/osgview_3d.jpeg)

## Contents

- Folder "cmake_modules": Contains cmake find scripts
- Folder "conf": Contains configuration
- Folder "data": Contains icons, textures, maps,...
- Folder "doc": Contains documentation
- Folder "src": Contains source code
- Folder "utils": Contains scripts for manual CSV import
- CMakeLists.txt: CMake config file
- LICENSE: GPL license
- readme.md: This file

## Released Experimental Version v0.3.0-beta
- [Current Appimage](https://github.com/hpuhr/ATSDB/releases/download/v0.3.0-beta/ATSDB-v0.3.0_x86_64.AppImage)
- [User Manual](https://github.com/hpuhr/ATSDB/releases/download/v0.3.0-beta/user_manual_v0.3.0.pdf)

## YouTube Videos
- [Installation, Import & OSGView](https://youtu.be/hptJHQ5D9hs)
- [OSGView Display Options](https://youtu.be/vEoT88RGLQo)
- [Labeling, Highlighting and Filtering](https://youtu.be/2ewXrWU7KUE)


## Newsletter
If you are interested in our newsletter, please send a mail to atsdb@gmx.at with the subject "Register".

## Author
Helmut Puhr
Contact: atsdb@gmx.at

## Licenses
The source code is released under [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html)

The binary is released under [Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/), [Legal Text](https://creativecommons.org/licenses/by/4.0/legalcode)

While it is permitted to use the AppImage for commercial purposes, the used open-source libraries might still prohibit this without further permission. It is the responsibility of the user to inspect the user manual and confirm that their use cases are permitted under the referenced licenses.

Disclaimer
----------

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


