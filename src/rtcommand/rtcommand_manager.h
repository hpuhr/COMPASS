#ifndef RTCOMMANDRECEIVER_H
#define RTCOMMANDRECEIVER_H

#include "configurable.h"
#include "singleton.h"

#include <QThread>

class RTCommandManager : public QThread, public Singleton, public Configurable
{
public:
    virtual ~RTCommandManager();

    void shutdown();

    static RTCommandManager& instance()
    {
        static RTCommandManager instance;
        return instance;
    }

protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    unsigned int port_num_ {27960};

    RTCommandManager();

private:
    void run();
};

#endif // RTCOMMANDRECEIVER_H
