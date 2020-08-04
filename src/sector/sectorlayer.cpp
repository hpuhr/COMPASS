#include "sectorlayer.h"
#include "sector.h"

#include <cassert>

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
    return sectors_.count(name);
}

void SectorLayer::addSector (std::shared_ptr<Sector> sector)
{
    assert (!hasSector(sector->name()));
    assert (sector->layerName() == name_);
    sectors_[sector->name()] = sector;
}

std::shared_ptr<Sector> SectorLayer::sector (const std::string& name)
{
    assert (hasSector(name));
    return sectors_.at(name);
}

void SectorLayer::removeSector (std::shared_ptr<Sector> sector)
{
    assert (hasSector(sector->name()));
    sectors_.erase(sector->name());
    assert (!hasSector(sector->name()));
}
