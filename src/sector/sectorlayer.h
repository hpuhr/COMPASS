#ifndef SECTORLAYER_H
#define SECTORLAYER_H

#include <string>
#include <vector>
#include <memory>

class Sector;
class EvaluationTargetPosition;

class SectorLayer
{
public:
    SectorLayer(const std::string& name);

    std::string name() const;

    bool hasSector (const std::string& name);
    void addSector (std::shared_ptr<Sector> sector);
    std::shared_ptr<Sector> sector (const std::string& name);
    void removeSector (std::shared_ptr<Sector> sector);

    unsigned int size () { return sectors_.size(); };

    std::vector<std::shared_ptr<Sector>>& sectors() { return sectors_; }

    bool isInside(const EvaluationTargetPosition& pos) const;

    std::pair<double, double> getMinMaxLatitude() const;
    std::pair<double, double> getMinMaxLongitude() const;

protected:
    const std::string name_;

    std::vector<std::shared_ptr<Sector>> sectors_;
};

#endif // SECTORLAYER_H
