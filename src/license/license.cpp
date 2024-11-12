/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "license.h"

#include "global.h"
#include "logger.h"
#include "files.h"
#include "timeconv.h"

#include <cassert>

#include <QString>
#include <QByteArray>
#include <QColor>

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/err.h>

#if USE_EXPERIMENTAL_SOURCE == true
#include "extra_resources.h"
#endif

namespace license
{

const std::string License::ColorValid   = QColor(34,139,34).name().toStdString();
const std::string License::ColorInvalid = QColor(220,20,60).name().toStdString();

const boost::posix_time::time_duration D0 = boost::posix_time::hours(24);
const boost::posix_time::time_duration D1 = boost::posix_time::hours(48);

/**
*/
std::string License::typeToString(Type t)
{
    if (t == Type::Free)
        return "Free";
    else if (t == Type::Trial)
        return "Trial";
    else if (t == Type::Pro)
        return "Pro";
    
    return "";
}

/**
*/
boost::optional<License::Type> License::typeFromString(const std::string& s)
{
    if (s == "Free")
        return Type::Free;
    else if (s == "Trial")
        return Type::Trial;
    else if (s == "Pro")
        return Type::Pro;
    
    return {};
}

/**
*/
std::string License::componentToString(Component c)
{
    if (c == ComponentProbIMMReconstructor)
        return "ProbIMMReconstructor";
    
    return "";
}

/**
*/
boost::optional<License::Component> License::componentFromString(const std::string& s)
{
    if (s == "ProbIMMReconstructor")
        return ComponentProbIMMReconstructor;

    return {};
}

/**
*/
boost::optional<nlohmann::json> License::unpackageLicenseKey(const std::string& license_key)
{
    auto bytes = QByteArray::fromHex(QString::fromStdString(license_key).toUtf8());
    std::vector<std::uint8_t> msgpack(bytes.size());
    memcpy(msgpack.data(), bytes.data(), (size_t)bytes.size() * sizeof(std::uint8_t));

    nlohmann::json j;
    
    try
    {
        j = nlohmann::json::from_msgpack(msgpack);
    }
    catch(...)
    {
        return {};
    }
    
    if (!j.is_object())
        return {};

    return j;
}

/**
*/
std::vector<unsigned char> License::licenseBytes(const nlohmann::json& license_json)
{
    if (!license_json.contains("license"))
        return {};

    const nlohmann::json& j = license_json.at("license");

    //license to msgpack
    auto byte_arr = nlohmann::json::to_msgpack(j);

    std::vector<unsigned char> bytes;
    bytes.resize(byte_arr.size());
    memcpy(bytes.data(), byte_arr.data(), byte_arr.size() * sizeof(unsigned char));

    return bytes;

        
}

/**
*/
std::vector<unsigned char> License::signatureBytes(const std::string& signature)
{
    //hey string to bytes
    auto byte_arr = QByteArray::fromHex(QByteArray::fromStdString(signature));

    std::vector<unsigned char> bytes;
    bytes.resize(byte_arr.count());
    memcpy(bytes.data(), byte_arr.data(), (size_t)byte_arr.count() * sizeof(unsigned char));

    return bytes;
}

/**
*/
std::string License::stringFromState(State state)
{
    if (state == State::Empty)
        return "Empty";
    else if (state == State::Invalid)
        return "Invalid";
    else if (state == State::Expired)
        return "Expired";
    else if (state == State::Valid)
        return "Valid";

    return "";
}

/**
*/
std::string License::colorFromState(State state)
{
    if (state == State::Empty)
        return ColorInvalid;
    else if (state == State::Invalid)
        return ColorInvalid;
    else if (state == State::Expired)
        return ColorInvalid;
    else if (state == State::Valid)
        return ColorValid;

    return "";
}

/**
*/
bool License::isComplete() const
{
    return (!id.empty() &&
            !signature.empty() &&
            json_blob.is_object());
}

/**
*/
std::pair<bool, std::string> License::read(const nlohmann::json& license_json)
{
    *this = {};

    try
    {
        if (!license_json.contains("license") ||
            !license_json.contains("signature") || 
            !license_json.contains("id"))
            return std::make_pair(false, "license structure invalid");

        signature = license_json.at("signature");
        if (signature.empty())
            return std::make_pair(false, "license signature missing");

        id = license_json.at("id");

        const auto& j = license_json.at("license");
        if (!j.is_object())
            return std::make_pair(false, "license structure invalid");

        if (!j.contains("version"))
            return std::make_pair(false, "license version missing");

        version = j.at("version");
        if (version < 1)
            return std::make_pair(false, "license version invalid");

        if (version >= 1)
        {
            if (!j.contains("type") ||
                !j.contains("components") ||
                !j.contains("licensee") ||
                !j.contains("date_activation") ||
                !j.contains("date_expiration") ||
                !j.contains("created") ||
                !j.contains("info"))
                return std::make_pair(false, "license structure invalid");

            //read type
            {
                if (!typeFromString(j.at("type")).has_value())
                    return std::make_pair(false, "license type invalid");
                
                type = typeFromString(j.at("type")).value();
            }

            //read components
            {
                const auto& components_json = j.at("components");
                if (!components_json.is_array())
                    return std::make_pair(false, "component list invalid");

                for(const auto& c : components_json)
                {
                    auto ctype = componentFromString(c);
                    if (!ctype.has_value())
                        return std::make_pair(false, "component list invalid");

                    components.insert(ctype.value());
                }
            }

            licensee = j.at("licensee");

            date_activation = Utils::Time::fromDateString(j["date_activation"]);
            date_expiration = Utils::Time::fromDateString(j["date_expiration"]);
            created         = Utils::Time::fromString(j["created"]);

            info = j.at("info");
        }
        else
        {
            return std::make_pair(false, "license version unknown");
        }

        json_blob = license_json;
    }
    catch(const std::exception& e)
    {
        return std::make_pair(false, e.what());
    }
    catch(...)
    {
        return std::make_pair(false, "unknown error");
    }
    
    return std::make_pair(true, "");
}

/**
*/
std::pair<bool, std::string> License::read(const std::string& license_key)
{
    *this = {};

    auto j = License::unpackageLicenseKey(license_key);
    if (!j.has_value())
        return std::make_pair(false, "key expansion failed");

    return read(j.value());
}

namespace
{
    EVP_PKEY* load_public_key(const std::string& public_key_str) 
    {
        BIO* bio = BIO_new_mem_buf(public_key_str.data(), -1);
        if (!bio)
            return nullptr;

        EVP_PKEY* pubkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);

        return pubkey;
    }
}

/**
*/
std::pair<bool, std::string> License::verify() const
{
#if USE_EXPERIMENTAL_SOURCE == true

    //get needed bytes
    auto bytes_data      = License::licenseBytes(json_blob);
    auto bytes_signature = License::signatureBytes(signature);

    if (bytes_data.empty())
        return std::make_pair(false, "invalid verification data bytes");
    if (bytes_signature.empty())
        return std::make_pair(false, "invalid verification signature bytes");
    
    // Load the public key
    EVP_PKEY* pubkey = load_public_key(ExtraResources::VerificationKey);
    if (!pubkey)
        return std::make_pair(false, "could not load key");
    
    // Create the context for verification
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) 
    {
        EVP_PKEY_free(pubkey);
        return std::make_pair(false, "error creating EVP_MD_CTX");
    }

    // Initialize the verification context with SHA-256 and PSS padding
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pubkey) != 1) 
    {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pubkey);
        return std::make_pair(false, "error initializing DigestVerify");
    }

    // Set up the PSS padding with MGF1 and maximum salt length
    if (EVP_PKEY_CTX_set_rsa_padding(EVP_MD_CTX_pkey_ctx(ctx), RSA_PKCS1_PSS_PADDING) <= 0) 
    {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pubkey);
        return std::make_pair(false, "error setting RSA PSS padding");
    }

    // -2 sets salt length according to block structure
    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(EVP_MD_CTX_pkey_ctx(ctx), -2) <= 0) 
    {  
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pubkey);
        return std::make_pair(false, "error setting PSS salt length");
    }

    // Verify the data (hash it and prepare for signature verification)
    if (EVP_DigestVerifyUpdate(ctx, bytes_data.data(), bytes_data.size()) != 1) 
    {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pubkey);
        return std::make_pair(false, "error updating DigestVerify");
    }

    // Finalize the verification (checks the signature)
    int verification_status = EVP_DigestVerifyFinal(ctx, bytes_signature.data(), bytes_signature.size());

    // Clean up
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pubkey);

    if (verification_status == 0)
        return std::make_pair(false, "license invalid");
    else if (verification_status == 1)
        return std::make_pair(true, "");

    return std::make_pair(false, "error during DigestVerifyFinal");

#else
    return std::make_pair(false, "public key missing");
#endif
}

/**
*/
std::pair<License::State, std::string> License::state() const
{
    if (!json_blob.is_object())
        return std::make_pair(State::Empty, "license empty");
    
    auto vres = verify();
    if (!vres.first)
        return std::make_pair(State::Invalid, vres.second);

    auto t = Utils::Time::currentUTCTime();

    if (t < date_activation - D0 || t > date_expiration + D1)
        return std::make_pair(State::Expired, "license expired");

    return std::make_pair(State::Valid, "");
}

/**
*/
std::string License::componentsAsString() const
{
    std::string component_str;
    for (const auto& c : components)
        component_str += (component_str.empty() ? "" : ", ") + componentToString(c);

    return component_str;
}

/**
*/
void License::print() const
{
    loginf << "version:    " << version;
    loginf << "type:       " << typeToString(type);
    loginf << "id:         " << id;
    loginf << "components: " << componentsAsString();
    loginf << "licensee:   " << licensee;
    loginf << "activates:  " << Utils::Time::toDateString(date_activation);
    loginf << "expires:    " << Utils::Time::toDateString(date_expiration);
    loginf << "created:    " << Utils::Time::toString(created);
    loginf << "info:       " << info;
    loginf << "signature:  " << signature;
}

}
