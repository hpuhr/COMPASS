#ifndef STOREDDBODATASOURCE_H
#define STOREDDBODATASOURCE_H

#include "configurable.h"

#include <string>
#include <memory>

#include <QWidget>

class DBObject;
class DBODataSource;
class StoredDBODataSourceWidget;

class StoredDBODataSource : public Configurable
{
public:
    StoredDBODataSource(const std::string& class_id, const std::string& instance_id, DBObject* object);
    StoredDBODataSource() = default;
    virtual ~StoredDBODataSource ();

    // copy from dbds, everything but id
    StoredDBODataSource& operator=(DBODataSource& other);
    /// @brief Move constructor
    StoredDBODataSource& operator=(StoredDBODataSource&& other);

    unsigned int id() const;
    //void id(unsigned int id);

    void name(const std::string &name);
    const std::string &name() const;

    void shortName(const std::string &short_name);
    const std::string &shortName() const;

    void sac(unsigned char sac);
    unsigned char sac() const;

    void sic(unsigned char sic);
    unsigned char sic() const;

    void latitude(double latitiude);
    double latitude() const;

    void longitude(double longitude_);
    double longitude() const;

    void altitude(double altitude);
    double altitude() const;

    StoredDBODataSourceWidget* widget (bool add_headers=false, QWidget* parent=0, Qt::WindowFlags f=0);

    DBObject& object () { assert (object_); return *object_; }

private:
    DBObject* object_ {nullptr};

    unsigned int id_{0};
    std::string name_;
    std::string short_name_;
    unsigned int sac_ {0};
    unsigned int sic_ {0};
    double latitude_ {0}; //degrees
    double longitude_ {0}; // degrees
    double altitude_ {0};  // meter above msl

    std::unique_ptr<StoredDBODataSourceWidget> widget_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // STOREDDBODATASOURCE_H
