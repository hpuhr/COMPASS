
#pragma once

#include <functional>

#include <QString>

class QMenu;

/**
*/
class TestLab
{
public:
    typedef std::function<bool()> TestFunc;

    TestLab(const QString& lab_name);
    virtual ~TestLab() = default;

protected:
    virtual void addTestsToMenu_impl(QMenu* menu) = 0;

    static void addTest(QMenu* menu, const QString& name, const TestFunc& func);

private:
    friend class TestLabCollection;

    void addTestsToMenu(QMenu* menu);

    QString lab_name_;
};

/**
*/
class TestLabCollection
{
public:
    TestLabCollection() = default;
    virtual ~TestLabCollection() = default;

    void appendTestLabs(QMenu* menu);
};
