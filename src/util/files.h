#ifndef FILES_H
#define FILES_H

#include <QDir>

#include "global.h"

static const std::string SYSTEM_INSTALL_PATH = CMAKE_INSTALL_PREFIX + std::string("/atsdb/");

static const std::string HOME_PATH = QDir::homePath().toStdString();

static const std::string HOME_SUBDIRECTORY = HOME_PATH + "/.atsdb/";
static const std::string CONF_SUBDIRECTORY = "conf/";
static const std::string DATA_SUBDIRECTORY = "data/";

static const std::string HOME_CONF_DIRECTORY = HOME_SUBDIRECTORY + CONF_SUBDIRECTORY;
static const std::string HOME_DATA_DIRECTORY = HOME_SUBDIRECTORY + DATA_SUBDIRECTORY;

extern std::string CURRENT_CONF_DIRECTORY;

namespace Utils
{
namespace Files
{
bool fileExists(const std::string& path);
void verifyFileExists(const std::string& path);
bool directoryExists(const std::string& path);
bool copyRecursively(const std::string& source_folder, const std::string& dest_folder);
QStringList getFilesInDirectory(const std::string& path);

std::string getIconFilepath(const std::string& filename);

void deleteFile(const std::string& filename);
void deleteFolder(const std::string& path);

std::string getDirectoryFromPath (const std::string& path);
std::string getFilenameFromPath (const std::string& path);

}  // namespace Files
}  // namespace Utils

#endif  // FILES_H
