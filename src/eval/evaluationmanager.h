#ifndef EVALUATIONMANAGER_H
#define EVALUATIONMANAGER_H

#include <QObject>

#include "configurable.h"

class ATSDB;

class EvaluationManager : public QObject, public Configurable
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb);
    virtual ~EvaluationManager();

    void close();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

protected:
    ATSDB& atsdb_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONMANAGER_H
