#ifndef MYSQLDBIMPORTJOB_H
#define MYSQLDBIMPORTJOB_H

#include "job.h"

class MySQLppConnection;

class MySQLDBImportJob : public Job
{
    Q_OBJECT
signals:
    void statusSignal (std::string message);

public:
    MySQLDBImportJob(const std::string& filename, bool archive, MySQLppConnection& connection);
    virtual ~MySQLDBImportJob();

    virtual void run ();

    size_t numLines() const;
    size_t numErrors() const;
    bool quitBecauseOfErrors() const;

private:
    std::string filename_;
    bool archive_ {false};

    MySQLppConnection& connection_;

    size_t num_lines_ {0};
    size_t num_errors_ {0};
    bool quit_because_of_errors_ {false};

    void importSQLFile ();
    void importSQLArchiveFile ();
};

#endif // MYSQLDBIMPORTJOB_H
