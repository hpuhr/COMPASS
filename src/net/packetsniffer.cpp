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

#include "packetsniffer.h"
#include "logger.h"
#include "files.h"

#include <iostream>
#include <cassert>
#include <sstream>

#include <pcap.h>
#include <pcap/sll.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <QString>
#include <QStringList>

const std::string PacketSniffer::SignatureStringSeparator = " => ";
const std::string PacketSniffer::SignatureIPPortSeparator = ":";

/**
*/
PacketSniffer::PacketSniffer() = default;

/**
*/
PacketSniffer::~PacketSniffer()
{
    //close any opened pcap file
    closePCAPFile();
}

/**
*/
void PacketSniffer::clear()
{
    num_read_         = 0;
    bytes_read_       = 0;
    num_read_total_   = 0;
    bytes_read_total_ = 0;
    packet_idx_       = 0;

    data_per_signature_.clear();
    data_ = {};

    unknown_link_types_.clear();
    unknown_eth_types_.clear();
    unknown_ip_prot_.clear();
}

/**
*/
std::pair<size_t, std::set<int>> PacketSniffer::unknownLinkTypes() const
{
    return std::make_pair(unknown_link_types_.size(), std::set<int>(unknown_link_types_.begin(), unknown_link_types_.end()));
}

/**
*/
std::pair<size_t, std::set<int>> PacketSniffer::unknownEthernetTypes() const
{
    return std::make_pair(unknown_eth_types_.size(), std::set<int>(unknown_eth_types_.begin(), unknown_eth_types_.end()));
}

/**
*/
std::pair<size_t, std::set<int>> PacketSniffer::unknownIPProtocols() const
{
    return std::make_pair(unknown_ip_prot_.size(), std::set<int>(unknown_ip_prot_.begin(), unknown_ip_prot_.end()));
}

/**
*/
bool PacketSniffer::hasUnknownPacketHeaders() const
{
    return (!unknown_link_types_.empty() ||
            !unknown_eth_types_.empty()  ||
            !unknown_ip_prot_.empty());
}

/**
*/
bool PacketSniffer::chunkEnded(const ReadConfig& read_config) const
{
    //check filters
    if (packet_idx_ < read_config.start_index || packet_idx_ > read_config.end_index)
        return true;
    if (num_read_ >= read_config.max_packets)
        return true;
    if (bytes_read_ >= read_config.max_bytes)
        return true;

    return false;
}

/**
*/
void PacketSniffer::digestPCAPPacket(const struct pcap_pkthdr* pkthdr, 
                                     const u_char* packet,
                                     int link_layer_type,
                                     const ReadConfig& read_config,
                                     bool& chunk_ended)
{
    //chunk already ended?
    if (chunk_ended)
        return;

    //skip packet?
    chunk_ended = chunkEnded(read_config);
    if (chunk_ended)
        return;
    
    if (link_layer_type == DLT_EN10MB)
    {
        const struct ether_header* eh = (struct ether_header*)packet;
        digestPCAPEtherPacket(ntohs(eh->ether_type), pkthdr, packet + sizeof(struct ether_header), sizeof(struct ether_header), read_config);
    }
    else if(link_layer_type == DLT_LINUX_SLL)
    {
        const struct sll_header* sll = (struct sll_header*)packet;
        digestPCAPEtherPacket(ntohs(sll->sll_protocol), pkthdr, packet + sizeof(struct sll_header), sizeof(struct sll_header), read_config);
    }
    else
    {
        unknown_link_types_.push_back(link_layer_type);
    }
}

/**
*/
void PacketSniffer::digestPCAPEtherPacket(int ether_type, 
                                          const struct pcap_pkthdr* pkthdr, 
                                          const u_char* packet, 
                                          unsigned long data_offs,
                                          const ReadConfig& read_config)
{
    const struct ip*      ipHeader;
    const struct tcphdr* tcpHeader;
    const struct udphdr* udpHeader;

    char  sourceIP[INET_ADDRSTRLEN];
    char  destIP  [INET_ADDRSTRLEN];
    u_int sourcePort, destPort;

    u_char* data       = nullptr;
    size_t  dataLength = 0;

    if (ether_type == ETHERTYPE_IP) 
    {
        ipHeader = (struct ip*)packet;
        inet_ntop(AF_INET, &(ipHeader->ip_src), sourceIP, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipHeader->ip_dst), destIP  , INET_ADDRSTRLEN);

        if (ipHeader->ip_p == IPPROTO_TCP) 
        {
            tcpHeader  = (struct tcphdr*)(packet + sizeof(struct ip));
            sourcePort = ntohs(tcpHeader->source);
            destPort   = ntohs(tcpHeader->dest);

            data       = (u_char*)(packet + sizeof(struct ip) + sizeof(struct tcphdr));
            dataLength = pkthdr->len - (data_offs + sizeof(struct ip) + sizeof(struct tcphdr));

            logdbg << "TCP Packet " << sourceIP << ":" << sourcePort << " => " << destIP << ":" << destPort << " = " << dataLength << " byte(s)";
        } 
        else if (ipHeader->ip_p == IPPROTO_UDP) 
        {
            udpHeader  = (struct udphdr*)(packet + sizeof(struct ip));
            sourcePort = ntohs(udpHeader->source);
            destPort   = ntohs(udpHeader->dest);

            data       = (u_char*)(packet + sizeof(struct ip) + sizeof(struct udphdr));
            dataLength = pkthdr->len - (data_offs + sizeof(struct ip) + sizeof(struct udphdr));

            logdbg << "UDP Packet " << sourceIP << ":" << sourcePort << " => " << destIP << ":" << destPort << " = " << dataLength << " byte(s)";
        }
        else
        {
            unknown_ip_prot_.push_back(ipHeader->ip_p);
        }
    }
    else
    {
        unknown_eth_types_.push_back(ether_type);
    }

    //supported packet?
    if (!data)
        return;

    addPacket(sourceIP, sourcePort, destIP, destPort, data, dataLength, read_config);
}

/**
*/
void PacketSniffer::addPacket(const std::string& src_ip,
                              unsigned int src_port,
                              const std::string& dst_ip,
                              unsigned int dst_port,
                              u_char* data,
                              size_t data_len,
                              const ReadConfig& read_config)
{
    Signature sig(src_ip, src_port, dst_ip, dst_port);

    //check signature filter
    if (!read_config.signatures_to_read.empty() && 
         read_config.signatures_to_read.count(sig) == 0)
         return;

    //log encountered signature in any case
    auto& sig_data = data_per_signature_[ sig ];

    //collect data
    auto& tdata = read_config.read_style == ReadStyle::PerSignature ? sig_data : data_;

    tdata.packets += 1;
    tdata.size    += data_len;

    tdata.data.insert(tdata.data.end(), data, data + data_len);

    num_read_         += 1;
    bytes_read_       += data_len;
    num_read_total_   += 1;
    bytes_read_total_ += data_len;
}

/**
*/
std::string PacketSniffer::signatureToString(const Signature& signature)
{
    return (std::get<0>(signature) + ":" + std::to_string(std::get<1>(signature)) + SignatureStringSeparator +
            std::get<2>(signature) + ":" + std::to_string(std::get<3>(signature))); 
}

/**
*/
PacketSniffer::Signature PacketSniffer::signatureFromString(const std::string& str)
{
    auto parts = QString::fromStdString(str).split(QString::fromStdString(SignatureStringSeparator));
    assert(parts.count() == 2&& !parts[ 0 ].isEmpty() && !parts[ 1 ].isEmpty());

    auto splitIPPort = [ & ] (const QString& ip_port_str)
    {
        auto ip_port = ip_port_str.split(QString::fromStdString(SignatureIPPortSeparator));
        assert(ip_port.count() == 2 && !ip_port[ 0 ].isEmpty() && !ip_port[ 1 ].isEmpty());

        bool ok = false;
        unsigned int port = ip_port[ 1 ].toUInt(&ok);
        assert(ok);

        return std::make_pair(ip_port[ 0 ].toStdString(), port);
    };

    auto ip_port0 = splitIPPort(parts[ 0 ]);
    auto ip_port1 = splitIPPort(parts[ 1 ]);

    return Signature(ip_port0.first, ip_port0.second, ip_port1.first, ip_port1.second);
}

/**
*/
void PacketSniffer::print() const
{
    std::cout << "=====================================================================" << std::endl;
    std::cout << "num packets read:       " << numPacketsRead() << std::endl;
    std::cout << "num bytes read:         " << numBytesRead() << std::endl;
    std::cout << "num packets read total: " << numPacketsReadTotal() << std::endl;
    std::cout << "num bytes read total:   " << numBytesReadTotal() << std::endl;
    std::cout << std::endl;
    
    auto printData = [ & ] (const std::string& sig_str, const Data& data)
    {
        std::cout << "    " << sig_str << ": "
                            << data.packets << " packet(s) "
                            << data.size << " byte(s) [" << data.data.size() << "]"
                            << std::endl;
    };

    std::cout << "encountered signatures:" << std::endl;

    for (const auto& d : dataPerSignature())
        printData(PacketSniffer::signatureToString(d.first), d.second);

    std::cout << std::endl;
    std::cout << "encountered data:" << std::endl;

    printData("data", data_);

    std::cout << std::endl;

    auto printUnknown = [ & ] (const std::pair<size_t, std::set<int>>& unknown, const std::string& name)
    {
        std::stringstream ss;
        ss << "encountered unknown " << name << ": " << unknown.first;
        if (unknown.second.size() > 0)
        {
            ss << " | ";
            for (auto u : unknown.second)
                ss << u << " ";
        }
        std::cout << ss.str() << std::endl;
    };

    printUnknown(unknownLinkTypes()    , "link types"    );
    printUnknown(unknownEthernetTypes(), "ethernet types");
    printUnknown(unknownIPProtocols()  , "ip protocols"  );
}

namespace
{
    /**
    */
    struct SnifferConfig
    {
        PacketSniffer*            sniffer         = nullptr;
        int                       link_layer_type = -1;
        PacketSniffer::ReadConfig read_config;
        bool                      chunk_ended     = false;
    };

    /**
    */
    void pcapPacketHandler(u_char *userData, 
                           const struct pcap_pkthdr* pkthdr, 
                           const u_char* packet)
    {
        //ugly cast ahead
        SnifferConfig* config = (SnifferConfig*)userData;

        assert(config->sniffer);

        //digest packet
        config->sniffer->digestPCAPPacket(pkthdr, packet, config->link_layer_type, config->read_config, config->chunk_ended);
    }
}

/**
*/
void PacketSniffer::closePCAPFile()
{
    if (pcap_file_)
    {
        pcap_close(pcap_file_);
        pcap_file_ = nullptr;

        if (device_ == Device::File)
            device_ = Device::NoDevice;
    }
}

/**
*/
bool PacketSniffer::openPCAP(const std::string& fn)
{
    clear();
    closePCAPFile();

    char errbuf[PCAP_ERRBUF_SIZE];

    pcap_file_ = pcap_open_offline(fn.c_str(), errbuf);
    if (pcap_file_ == NULL)
    {
        logerr << "PacketSniffer: openPCAP: open pcap file '" << Utils::Files::getFilenameFromPath(fn) << "' failed";
        return false;
    }

    //get link layer type
    link_layer_type_ = pcap_datalink(pcap_file_);

    device_ = Device::File;

    loginf << "PacketSniffer: openPCAP: opened pcap file '" << Utils::Files::getFilenameFromPath(fn) << "'" << " with link layer type " << link_layer_type_;

    return true;
 }

/**
*/
bool PacketSniffer::readFile(ReadStyle read_style,
                             size_t max_packets,
                             size_t max_bytes,
                             size_t start_index,
                             size_t end_index,
                             const std::set<Signature>& signatures_to_read)
{
    clear();

    if (device_ != Device::File || pcap_file_ == nullptr)
    {
        logerr << "PacketSniffer: read: no file device opened";
        return false;
    }

    //config to be passed to packet handler
    SnifferConfig config;

    config.sniffer            = this;
    config.link_layer_type    = link_layer_type_;

    config.read_config.read_style         = read_style;
    config.read_config.start_index        = start_index;
    config.read_config.end_index          = end_index;
    config.read_config.max_packets        = max_packets;
    config.read_config.max_bytes          = max_bytes;
    config.read_config.signatures_to_read = signatures_to_read;

    //loop packets
    if (pcap_loop(pcap_file_, 0, pcapPacketHandler, (u_char*)&config) < 0) 
        return false;

    return true;
}

/**
*/
boost::optional<PacketSniffer::Data> PacketSniffer::readFileNext(size_t max_packets, 
                                                                 size_t max_bytes, 
                                                                 const std::set<Signature>& signatures_to_read)
{
    if (device_ != Device::File || pcap_file_ == nullptr)
    {
        logerr << "PacketSniffer: readFileNext: no file device opened";
        return {};
    }

    //config to be passed to packet handler
    SnifferConfig config;

    config.sniffer            = this;
    config.link_layer_type    = link_layer_type_;

    config.read_config.read_style         = ReadStyle::Accumulate;
    config.read_config.max_packets        = max_packets;
    config.read_config.max_bytes          = max_bytes;
    config.read_config.signatures_to_read = signatures_to_read;

    //reset counts
    num_read_   = 0;
    bytes_read_ = 0;
    data_       = {};

    pcap_pkthdr** pkthdr;
    const uchar** packet;

    bool error = false;

    while (!config.chunk_ended)
    {
        int ret = pcap_next_ex(pcap_file_, pkthdr, packet);

        //file ended?
        if (ret == PCAP_ERROR_BREAK)
            break;
        
        //read error?
        if (ret != 1)
        {
            error = true;
            break;
        }
    }

    if (error)
        return {};

    return data_;
}
