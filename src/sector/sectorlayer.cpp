#include "sectorlayer.h"
#include "sector.h"

#include <cassert>

using namespace std;

SectorLayer::SectorLayer(const std::string& name)
  : name_(name)
{

}

std::string SectorLayer::name() const
{
    return name_;
}

bool SectorLayer::hasSector (const std::string& name)
{
    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});

    return iter != sectors_.end();
}

void SectorLayer::addSector (std::shared_ptr<Sector> sector)
{
    assert (!hasSector(sector->name()));
    assert (sector->layerName() == name_);
    sectors_.push_back(sector);
}

std::shared_ptr<Sector> SectorLayer::sector (const std::string& name)
{
    assert (hasSector(name));

    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});
    assert (iter != sectors_.end());

    return *iter;
}

void SectorLayer::removeSector (std::shared_ptr<Sector> sector)
{
    assert (hasSector(sector->name()));

    string name = sector->name();
    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});
    assert (iter != sectors_.end());

    sectors_.erase(iter);
    assert (!hasSector(sector->name()));
}
