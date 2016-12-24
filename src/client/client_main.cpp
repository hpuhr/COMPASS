#include "Config.h"
#include "Logger.h"

#include "Buffer.h"
#include "ArrayList.h"

#include "boost/date_time/posix_time/posix_time.hpp"

int main (int argc, char **argv)
{
    Config::getInstance().init ("conf/client.conf");
    Logger::getInstance();

    loginf << "Main: Startup";

    try
    {
        unsigned int test_size=10000000;
        boost::posix_time::ptime start_time;
        boost::posix_time::ptime stop_time;

        loginf << "Main: Testing arraylist double";

        start_time= boost::posix_time::microsec_clock::local_time();

        ArrayListTemplate<double> list;

        for (unsigned int cnt=0; cnt < test_size; cnt++)
            list.set(cnt, 1.0);

        for (unsigned int cnt=0; cnt < list.getSize(); cnt++)
        {
            assert (list.get(cnt) == 1.0);
            assert (list.isNone(cnt) == false);
        }


        list.clear();
        for (unsigned int cnt=0; cnt < list.getSize(); cnt++)
        {
            assert (list.isNone(cnt) == true);
        }

        stop_time= boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = stop_time - start_time;
        loginf  << "Main: list test done after " << diff.total_milliseconds() << "ms";

        loginf << "Main: Testing buffer double";

        start_time= boost::posix_time::microsec_clock::local_time();

        Buffer buffer;
        buffer.addProperty("my_bonny", PropertyDataType::DOUBLE);

        for (unsigned int cnt=0; cnt < test_size; cnt++)
            buffer.getDouble("my_bonny").set(cnt, 1.0);

        for (unsigned int cnt=0; cnt < buffer.getDouble("my_bonny").getSize(); cnt++)
        {
            assert (buffer.getDouble("my_bonny").get(cnt) == 1.0);
            assert (buffer.getDouble("my_bonny").isNone(cnt) == false);
        }


        buffer.getDouble("my_bonny").clear();
        for (unsigned int cnt=0; cnt < buffer.getDouble("my_bonny").getSize(); cnt++)
        {
            assert (buffer.getDouble("my_bonny").isNone(cnt) == true);
        }

        stop_time= boost::posix_time::microsec_clock::local_time();
        diff = stop_time - start_time;
        loginf  << "Main: buffer test done after " << diff.total_milliseconds() << "ms";

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
