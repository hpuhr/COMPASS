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

#pragma once

#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <limits>

#include <boost/optional.hpp>

struct pcap_pkthdr;
struct pcap;

/**
*/
class PacketSniffer
{
public:
    /**
    */
    struct Data
    {
        bool valid() const 
        {
            return (packets > 0 && size > 0 && data.size() > 0);
        }

        size_t packets = 0;
        size_t size    = 0;

        std::vector<u_char> data;
    };

    enum class Device
    {
        NoDevice = 0,
        File
    };

    enum class ReadStyle
    {
        Accumulate = 0,
        PerSignature
    };

    enum class Error
    {
        NoError = 0,
        OpenFailed,
        ParseFailed
    };

    typedef std::tuple<std::string, unsigned int, std::string, unsigned int> Signature;
    typedef std::map<Signature, Data>                                        DataPerSignature;

    /**
    */
    struct ReadConfig
    {
        ReadStyle           read_style      = PacketSniffer::ReadStyle::Accumulate;
        size_t              start_index     = 0;
        size_t              end_index       = std::numeric_limits<size_t>::max();
        size_t              max_packets     = std::numeric_limits<size_t>::max();
        size_t              max_bytes       = std::numeric_limits<size_t>::max();

        std::set<Signature> signatures_to_read;
    };
    
    PacketSniffer();
    virtual ~PacketSniffer();

    void clear();
    bool openPCAP(const std::string& fn);

    bool readFile(ReadStyle read_style = ReadStyle::Accumulate,
                  size_t max_packets = std::numeric_limits<size_t>::max(),
                  size_t max_bytes = std::numeric_limits<size_t>::max(),
                  size_t start_index = 0,
                  size_t end_index = std::numeric_limits<size_t>::max(),
                  const std::set<Signature>& signatures_to_read = std::set<Signature>());
    boost::optional<Data> readFileNext(size_t max_packets = 0, 
                                       size_t max_bytes = 0,
                                       const std::set<Signature>& signatures_to_read = std::set<Signature>());

    void digestPCAPPacket(const struct pcap_pkthdr* pkthdr,
                          const u_char* packet,
                          int link_layer_type,
                          const ReadConfig& read_config,
                          bool& chunk_ended);

    void print() const;

    size_t numPacketsRead() const { return num_read_; }
    size_t numBytesRead() const { return bytes_read_; }
    size_t numPacketsReadTotal() const { return num_read_total_; }
    size_t numBytesReadTotal() const { return bytes_read_total_; }

    const DataPerSignature& dataPerSignature() const { return data_per_signature_; }
    const Data& data() const { return data_; }

    std::pair<size_t, std::set<int>> unknownLinkTypes() const;
    std::pair<size_t, std::set<int>> unknownEthernetTypes() const;
    std::pair<size_t, std::set<int>> unknownIPProtocols() const;

    bool hasUnknownPacketHeaders() const;

    static std::string signatureToString(const Signature& sig);
    static Signature signatureFromString(const std::string& str);

    static const std::string SignatureStringSeparator;
    static const std::string SignatureIPPortSeparator;

private:
    void digestPCAPEtherPacket(int ether_type, 
                               const struct pcap_pkthdr* pkthdr, 
                               const u_char* packet, 
                               unsigned long data_offs,
                               const ReadConfig& read_config);
    void addPacket(const std::string& src_ip,
                   unsigned int src_port,
                   const std::string& dst_ip,
                   unsigned int dst_port,
                   u_char* data,
                   size_t data_len,
                   const ReadConfig& read_config);
    void closePCAPFile();

    bool chunkEnded(const ReadConfig& read_config) const;

    size_t packet_idx_       = 0;
    size_t num_read_         = 0;
    size_t bytes_read_       = 0;
    size_t num_read_total_   = 0;
    size_t bytes_read_total_ = 0;

    DataPerSignature data_per_signature_;
    Data             data_;

    std::vector<int> unknown_link_types_;
    std::vector<int> unknown_eth_types_;
    std::vector<int> unknown_ip_prot_;

    pcap*   pcap_file_       = nullptr;
    Device  device_          = Device::NoDevice;
    int     link_layer_type_ = -1;
};
