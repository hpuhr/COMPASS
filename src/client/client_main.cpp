#include "Config.h"
#include "Logger.h"

#include "ArrayList.h"

int main (int argc, char **argv)
{
    Config::getInstance().init ("conf/client.conf");
    Logger::getInstance();

    loginf << "Main: Startup";

    try
    {
        loginf << "Main: Testing array list bool";
        ArrayListTemplate<double> array_list;

        for (unsigned int cnt=0; cnt < 50000000; cnt++)
            array_list.set(cnt, 1.0);
        array_list.clear();
        for (unsigned int cnt=0; cnt < 50000000; cnt++)
        {
            assert (array_list.get(cnt) == 0.0);
            assert (array_list.isNone(cnt) == false);
        }

        array_list.setAllNone();
        for (unsigned int cnt=0; cnt < 50000000; cnt++)
        {
            assert (array_list.isNone(cnt) == true);
        }


        return 0;
    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";

        return -1;
    }
    loginf << "Main: Shutdown";
    return 0;
}
