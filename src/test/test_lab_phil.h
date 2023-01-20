
#pragma once

#include "test_lab.h"

/**
*/
class TestLabPhil : public TestLab
{
public:
    TestLabPhil();
    virtual ~TestLabPhil() = default;

protected:
    void addTestsToMenu_impl(QMenu* menu) override final;

private:
    static bool uiInjectionTest();
    static bool uiObjectNameGenerationTest();
    static bool uiSetTest();
};
