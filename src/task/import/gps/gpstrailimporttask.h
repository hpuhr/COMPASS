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
class Buffer;

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

    std::string dsName() const;
    void dsName(const std::string& ds_name);

    unsigned int dsSAC() const;
    void dsSAC(unsigned int sac);

    unsigned int dsSIC() const;
    void dsSIC(unsigned int sic);

protected:
    std::map<std::string, SavedFile*> file_list_;
    std::string current_filename_;

    std::string ds_name_;
    unsigned int ds_sac_ {0};
    unsigned int ds_sic_ {0};

    std::unique_ptr<GPSTrailImportTaskWidget> widget_;

    std::string current_error_;
    std::string current_text_;

    std::vector<nmea::GPSFix> gps_fixes_;
    //unsigned int gps_fixes_skipped_ {0};

    std::shared_ptr<Buffer> buffer_;

    virtual void checkSubConfigurables() {}

    void parseCurrentFile ();
    //void checkParsedData (); // throws exceptions for errors
};

#endif // GPSTRAILIMPORTTASK_H
