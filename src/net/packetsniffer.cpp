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

/**
*/
PacketSniffer::PacketSniffer() = default;

/**
*/
PacketSniffer::~PacketSniffer() = default;

/**
*/
void PacketSniffer::clear()
{
    num_read_ = 0;

    transactions_.clear();

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
void PacketSniffer::digestPCAPPacket(const struct pcap_pkthdr* pkthdr, 
                                     const u_char* packet,
                                     int link_layer_type)
{
    if (link_layer_type == DLT_EN10MB)
    {
        const struct ether_header* eh = (struct ether_header*)packet;
        digestPCAPEtherPacket(ntohs(eh->ether_type), pkthdr, packet + sizeof(struct ether_header), sizeof(struct ether_header));
    }
    else if(link_layer_type == DLT_LINUX_SLL)
    {
        const struct sll_header* sll = (struct sll_header*)packet;
        digestPCAPEtherPacket(ntohs(sll->sll_protocol), pkthdr, packet + sizeof(struct sll_header), sizeof(struct sll_header));
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
                                          unsigned long data_offs)
{
    const struct ip*           ipHeader;
    const struct tcphdr*       tcpHeader;
    const struct udphdr*       udpHeader;

    char  sourceIP[INET_ADDRSTRLEN];
    char  destIP  [INET_ADDRSTRLEN];
    u_int sourcePort, destPort;

    u_char* data = nullptr;
    size_t dataLength = 0;

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

            std::cout << "TCP Packet " << sourceIP << ":" << sourcePort << " => " << destIP << ":" << destPort << " = " << dataLength << " byte(s)" << std::endl;
        } 
        else if (ipHeader->ip_p == IPPROTO_UDP) 
        {
            udpHeader  = (struct udphdr*)(packet + sizeof(struct ip));
            sourcePort = ntohs(udpHeader->source);
            destPort   = ntohs(udpHeader->dest);

            data       = (u_char*)(packet + sizeof(struct ip) + sizeof(struct udphdr));
            dataLength = pkthdr->len - (data_offs + sizeof(struct ip) + sizeof(struct udphdr));

            std::cout << "UDP Packet " << sourceIP << ":" << sourcePort << " => " << destIP << ":" << destPort << " = " << dataLength << " byte(s)" << std::endl;
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

    

    addPacket(sourceIP, sourcePort, destIP, destPort, data, dataLength);
}

/**
*/
void PacketSniffer::addPacket(const std::string& src_ip,
                              unsigned int src_port,
                              const std::string& dst_ip,
                              unsigned int dst_port,
                              u_char* data,
                              size_t data_len)
{
    auto& tdata = transactions_[ Transaction(src_ip, src_port, dst_ip, dst_port) ];

    tdata.packets += 1;
    tdata.size    += data_len;

    tdata.data.insert(tdata.data.end(), data, data + data_len);

    num_read_   += 1;
    bytes_read_ += data_len;
}

namespace
{
    struct SnifferConfig
    {
        PacketSniffer* sniffer         = nullptr;
        int            link_layer_type = -1;
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
        config->sniffer->digestPCAPPacket(pkthdr, packet, config->link_layer_type);
    }
}

/**
*/
void PacketSniffer::print() const
{
    std::cout << "=====================================================================" << std::endl;
    std::cout << "num packets read: " << numRead() << std::endl;
    std::cout << "num bytes read:   " << numBytesRead() << std::endl;
    std::cout << std::endl;
    std::cout << "encountered transactions:" << std::endl;

    for (const auto& t : transactions())
    {
        std::cout << "    " << std::get<0>(t.first) << ":" << std::get<1>(t.first) << " => "
                            << std::get<2>(t.first) << ":" << std::get<3>(t.first) << ": "
                            << t.second.packets << " packet(s) "
                            << t.second.size << " byte(s) [" << t.second.data.size() << "]"
                            << std::endl;
    }
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

    printUnknown(unknownLinkTypes(), "link types");
    printUnknown(unknownEthernetTypes(), "ethernet types");
    printUnknown(unknownIPProtocols(), "ip protocols");
}

/**
*/
PacketSniffer::Error PacketSniffer::readPCAP(const std::string& fn)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    //open pcap file
    pcap_t* fp = pcap_open_offline(fn.c_str(), errbuf);
    if (fp == NULL) 
        return Error::OpenFailed;

    //get link layer type
    int link_layer_type = pcap_datalink(fp);

    std::cout << "link layer type: " << link_layer_type << std::endl;

    //config to be passed to packet handler
    SnifferConfig config;
    config.sniffer         = this;
    config.link_layer_type = link_layer_type;

    //loop packets
    if (pcap_loop(fp, 0, pcapPacketHandler, (u_char*)&config) < 0) 
        return Error::ParseFailed;

    return Error::NoError;
}
