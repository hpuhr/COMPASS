/*
 * Abd.h
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#ifndef ABD_H_
#define ABD_H_

#include <string>
#include <vector>

namespace ACGXML
{

struct AseUid
{
    unsigned int mid_;
    std::string code_type_;
    std::string code_id_;
};

struct Avx
{
    bool has_gbr_uid_;
    unsigned int gbr_mid_;
    std::string gbr_txt_name_;

    std::string code_type_;
    double geo_lat_;
    double geo_long_;
    std::string code_datum_;

    bool has_arc_;
    double geo_lat_arc_;
    double geo_long_arc_;
    double val_radius_arc_;
    std::string uom_radius_arc_;
};

class Abd
{
public:
    Abd();
    virtual ~Abd();

    unsigned int mid_;

    AseUid ase_uid_;
    std::vector <Avx> avxes_;
};

} /* namespace ACGXMLParser */

#endif /* ABD_H_ */
