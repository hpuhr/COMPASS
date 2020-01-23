#include "files.h"

#include <cassert>
#include <stdexcept>
#include <iostream>

#include <QFileInfo>
#include <QString>
#include <QDir>

std::string CURRENT_CONF_DIRECTORY;

namespace Utils
{

namespace Files
{

bool fileExists(const std::string& path)
{
    QFileInfo check_file(QString::fromStdString(path));
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void verifyFileExists(const std::string& path)
{
    if (!fileExists(path))
        throw std::runtime_error ("Utils: Files: verifyFileExists: file '" + path + "' does not exist");
}

bool directoryExists(const std::string&  path)
{
    QFileInfo check_file(QString::fromStdString(path));
    // check if file exists and if yes: Is it really a directory?
    return check_file.exists() && check_file.isDir();
}

bool copyRecursively(const std::string& source_folder, const std::string& dest_folder)
{
    QString sourceFolder (QString::fromStdString(source_folder));
    QString destFolder (QString::fromStdString(dest_folder));
    bool success = false;
    QDir sourceDir(sourceFolder);

    if(!sourceDir.exists())
    {
        std::cout << "Files: copyRecursively: source dir " << source_folder << " doesn't exist" << std::endl;
        return false;
    }

    QDir destDir(destFolder);
    if(!destDir.exists())
        destDir.mkdir(destFolder);


    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];

        if (QFile::exists(destName))
            QFile::remove(destName);

        success = QFile::copy(srcName, destName);
        if(!success)
        {
            std::cout << "Files: copyRecursively: file copy failed of " << srcName.toStdString()
                 << " to " << destName.toStdString() << std::endl;
            return false;
        }
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        success = copyRecursively(srcName.toStdString(), destName.toStdString());
        if(!success)
        {
            std::cout << "Files: copyRecursively: directory copy failed of " << srcName.toStdString()
                 << " to " << destName.toStdString()  << std::endl;
            return false;
        }
    }

    return true;
}

QStringList getFilesInDirectory (const std::string& path)
{
    assert (directoryExists(path));
    QDir directory(path.c_str());
    QStringList list = directory.entryList(QStringList({"*"}),QDir::Files); // << "*.jpg" << "*.JPG"
    return list;
}

std::string getIconFilepath (const std::string& filename)
{
    std::string filepath = HOME_DATA_DIRECTORY+"icons/"+filename;
    verifyFileExists (filepath);
    return filepath;
}

void deleteFolder (const std::string& path)
{
    QDir dir(path.c_str());
    dir.removeRecursively();
}

}
}
