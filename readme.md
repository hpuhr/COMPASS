## Description
Based on a dynamic database schema, recorded air traffic surveillance data can be inserted, retrieved and inspected.

- Support of multiple database systems, e.g. Sqlite3, MySQL
- Support of multiple, configurable database schemas, e.g. SCDB
- Dynamic JSON import from jASTERIX, SDDL, ADS-B exchange, OpenSky Network
- Dynamic ASTERIX import using [jASTERIX](https://github.com/hpuhr/jASTERIX)
- Import of (D)GPS trails from NMEA files
- Import of polygons from GML,KML,ESRI Shapefiles
- MySQL database import and management of SCDB databases
- High performance processing, low memory footprint
- Filtering for detailed analysis
- Simple custom filter generation
- ARTAS track association (TRI) analysis
- Supported Database Objects
  - Radar plots
  - MLAT & WAM target reports
  - ADS-B target reports
  - System Track updates
  - Reference trajectory updates
- Textual data inspection using Listbox View
  - Display of data as text tables
  - Configurable data loading of data of interest
  - Exporting of data as CSV
- Graphical data inspection using OSG View
  - Customizable map/terrain display based on osgEarth
  - Customizable display of ATC surveillance data
  - High-speed time-filtered display
  - Numerous operations for analysis, e.g. data selection, labeling, distance measurement
  - Configurable data layering and styling for detailed analysis
  - Relatively low memory footprint (e.g. 16 million target reports in ~8 GB RAM)
- Cross-view data selection and inspection
  - Command line options for automated processing
  - View points for efficient inspection
  - Standard compliance evaluation (under construction)
  - Definition of standards based on configurable requirements
  - Generalized comparison of test data vs. reference data
  - Calculation of requirements/performance indicators
  - Investigation/display of results on several levels of detail
  - Manual removal of specific targets possible
  - Export of results as report PDF


Please refer to the releases page for the user manual and the AppImage. Please do read the user manual before running the application.

## Released Experimental Version v0.5.5-beta
- [Current Appimage](https://github.com/hpuhr/ATSDB/releases/download/v0.5.5-beta/ATSDB-v0.5.5.AppImage)
- [User Manual](https://github.com/hpuhr/ATSDB/releases/download/v0.5.5-beta/user_manual_v0.5.5.pdf)
- [Improved Offline Map](https://github.com/hpuhr/ATSDB/releases/download/v0.5.4-beta/map_minimal_detailed.zip)

## Screenshots

![alt text](https://github.com/hpuhr/ATSDB/blob/master/doc/screenshots/app_ss1.png)

![alt text](https://github.com/hpuhr/ATSDB/blob/master/doc/screenshots/app_ss3.png)

![alt text](https://github.com/hpuhr/ATSDB/blob/master/doc/screenshots/app_ss2.png)

## YouTube Videos
### v0.5.0
- [v0.5.0: Import ASTERIX & Setup](https://youtu.be/o1S3S9tcifA)
- [v0.5.0: New OSGView Features](https://youtu.be/c1v3tIjNLVM)

### Old from v0.4.0
- [v0.4.0: Import ASTERIX & Setup](https://youtu.be/QIMVb9HNBJc)
- [v0.4.0: Basics](https://youtu.be/ny47qrBlyfM)
- [v0.4.0: Advanced Usage](https://youtu.be/_L65VO8TsyE)


## Contents

- appimage/: Contains AppImage build base
- cmake_modules/: Contains cmake find scripts
- conf/: Contains configuration
- data/: Contains icons, textures, maps,...
- doc/: Contains documentation
- docker/: Contains Docker buildfile and AppImage build scripts
- src/: Contains source code
- utils/": Contains scripts for manual CSV import
- CMakeLists.txt: CMake config file
- LICENSE: GPL license
- readme.md: This file

## Newsletter
If you are interested in our newsletter, please send a mail to compass@openats.at with the subject "Register".

## Author
Helmut Puhr
Contact: compass@openats.at

## Licenses
The source code is released under [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html)

The binary is released under [Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/), [Legal Text](https://creativecommons.org/licenses/by/4.0/legalcode)

While it is permitted to use the AppImage for commercial purposes, the used open-source libraries might still prohibit this without further permission. It is the responsibility of the user to inspect the user manual and confirm that their use cases are permitted under the referenced licenses.

Disclaimer
----------

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


