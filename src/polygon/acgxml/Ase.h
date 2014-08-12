/*
 * Ase.h
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#ifndef ASE_H_
#define ASE_H_

#include <string>

//        <AseUid mid="1395">
//            <codeType>RAS</codeType>
//            <codeId>LO61</codeId>
//        </AseUid>
//        <txtLocalType>MFA-ASR</txtLocalType>
//        <codeMil>CIVIL</codeMil>
//        <codeDistVerUpper>STD</codeDistVerUpper>
//        <valDistVerUpper>999</valDistVerUpper>
//        <uomDistVerUpper>FL</uomDistVerUpper>
//        <codeDistVerLower>ALT</codeDistVerLower>
//        <valDistVerLower>7700</valDistVerLower>
//        <uomDistVerLower>FT</uomDistVerLower>

//        <codeDistVerMnm>ALT</codeDistVerMnm>
//        <valDistVerMnm>5500</valDistVerMnm>
//        <uomDistVerMnm>FT</uomDistVerMnm>

//        <Att>
//            <codeWorkHr>H24</codeWorkHr>
//        </Att>

namespace ACGXML
{
class Ase
{
public:
    unsigned int uid_mid_;
    std::string uid_code_type_;
    std::string uid_code_id_;

    std::string txt_local_type_;
    std::string code_mil_;

    std::string code_dist_ver_upper_;
    unsigned int val_dist_ver_upper_;
    std::string uom_dist_ver_upper_;

    std::string code_dist_ver_lower_;
    unsigned int val_dist_ver_lower_;
    std::string uom_dist_ver_lower_;

    bool has_minimum_;
    std::string code_dist_ver_minimum_;
    double val_dist_ver_minimum_;
    std::string uom_dist_ver_minimum_;

    std::string code_work_hr_; // wörk wörk

    Ase();
    virtual ~Ase();
};
}
#endif /* ASE_H_ */
