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

struct pcap_pkthdr;

/**
*/
class PacketSniffer
{
public:
    struct TransactionData
    {
        size_t packets = 0;
        size_t size    = 0;

        std::vector<u_char> data;
    };

    enum class Error
    {
        NoError = 0,
        OpenFailed,
        ParseFailed
    };

    typedef std::tuple<std::string, unsigned int, std::string, unsigned int> Transaction;
    typedef std::map<Transaction, TransactionData>                           Transactions;
    
    PacketSniffer();
    virtual ~PacketSniffer();

    void clear();

    Error readPCAP(const std::string& fn);

    void digestPCAPPacket(const struct pcap_pkthdr* pkthdr,
                          const u_char* packet,
                          int link_layer_type);

    void print() const;

    size_t numRead() const { return num_read_; }
    size_t numBytesRead() const { return bytes_read_; }

    const Transactions& transactions() const { return transactions_; }

    std::pair<size_t, std::set<int>> unknownLinkTypes() const;
    std::pair<size_t, std::set<int>> unknownEthernetTypes() const;
    std::pair<size_t, std::set<int>> unknownIPProtocols() const;

private:
    void digestPCAPEtherPacket(int ether_type, 
                               const struct pcap_pkthdr* pkthdr, 
                               const u_char* packet, 
                               unsigned long data_offs);
    void addPacket(const std::string& src_ip,
                    unsigned int src_port,
                    const std::string& dst_ip,
                    unsigned int dst_port,
                    u_char* data,
                    size_t data_len);

    size_t num_read_   = 0;
    size_t bytes_read_ = 0;

    Transactions transactions_;

    std::vector<int> unknown_link_types_;
    std::vector<int> unknown_eth_types_;
    std::vector<int> unknown_ip_prot_;
};
