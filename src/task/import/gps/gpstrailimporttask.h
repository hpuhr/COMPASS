#ifndef GPSTRAILIMPORTTASK_H
#define GPSTRAILIMPORTTASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>
#include <memory>

#include <nmeaparse/nmea.h>

class TaskManager;
class GPSTrailImportTaskWidget;
class SavedFile;
class DBObject;

class GPSTrailImportTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void insertProgressSlot(float percent);
    void insertDoneSlot(DBObject& object);

public:
    GPSTrailImportTask(const std::string& class_id, const std::string& instance_id,
                       TaskManager& task_manager);
    virtual ~GPSTrailImportTask();

    virtual TaskWidget* widget();
    virtual void deleteWidget();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool canImportFile();
    virtual bool canRun();
    virtual void run();

    const std::map<std::string, SavedFile*>& fileList() { return file_list_; }
    bool hasFile(const std::string& filename) { return file_list_.count(filename) > 0; }
    void addFile(const std::string& filename);
    void removeCurrentFilename();
    void removeAllFiles ();
    void currentFilename(const std::string& filename);
    const std::string& currentFilename() { return current_filename_; }

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

    std::string currentText() const;
    std::string currentError() const;

protected:
    std::map<std::string, SavedFile*> file_list_;
    std::string current_filename_;

    std::unique_ptr<GPSTrailImportTaskWidget> widget_;

    std::string current_error_;
    std::string current_text_;

    std::vector<nmea::GPSFix> gps_fixes_;
    //unsigned int gps_fixes_skipped_ {0};

    virtual void checkSubConfigurables() {}

    void parseCurrentFile ();
    //void checkParsedData (); // throws exceptions for errors
};

#endif // GPSTRAILIMPORTTASK_H
