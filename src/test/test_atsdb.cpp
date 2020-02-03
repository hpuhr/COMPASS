#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "client.h"
#include "mainwindow.h"

//#include <thread>

//std::unique_ptr<Client> client_;

//void thread_function()
//{
//    int argc = 0;
//    char *argv[] = {"test_atsdb", NULL};

//    client_.reset(new Client (argc, argv));


//    if (client_->quitRequested())
//        return;

//    client_->mainWindow().show();
//    client_->exec();
//}

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

TEST_CASE( "Factorials are computed", "[factorial]" )
{
//    std::thread threadObj(thread_function);
//     for(int i = 0; i < 10000; i++)
//         std::cout<<"Display From MainThread"<<std::endl;

//     client_->mainWindow().close();

//     threadObj.join();

    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}
