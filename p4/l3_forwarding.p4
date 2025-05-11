/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>


//  Types and Constants
const bit<16> TYPE_IPV4 = 0x800;
const bit<8>  IP_PROTO = 253;

// HEADERS
typedef bit<48> macAddr_v;
typedef bit<32> ip4Addr_v;
typedef bit<9>  egressSpec_v;


header ethernet_h {
    macAddr_v   dstAddr;
    macAddr_v   srcAddr;
    bit<16>     etherType;
}

header ipv4_h {
    bit<4>      version;
    bit<4>      ihl;
    bit<8>      diffserv;
    bit<16>     totalLen;
    bit<16>     identification;
    bit<3>      flags;
    bit<13>     fragOffset;
    bit<8>      ttl;
    bit<8>      protocol;
    bit<16>     hdrChecksum;
    ip4Addr_v   srcAddr;
    ip4Addr_v   dstAddr;
}


struct headers_t {
    ethernet_h              ethernet;
    ipv4_h                  ipv4;
}

struct metadata_t {
    bit<16>     tcp_length; // TCP packet size in # of bytes to update checksum
    varbit<320> tcp_options_data;
}

parser MyParser(packet_in packet,
                out headers_t hdr,
                inout metadata_t meta,
                inout standard_metadata_t standard_metadata) {

    bit<7> tcp_hdr_bytes_left;

    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            TYPE_IPV4: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition accept;
    }
}   

control MyVerifyChecksum(inout headers_t hdr, inout metadata_t meta) {   
    apply {  }
}

control MyIngress(inout headers_t hdr,
                  inout metadata_t meta,
                  inout standard_metadata_t standard_metadata) {
    action drop() {
        mark_to_drop(standard_metadata);
    }
    
    action ipv4_forward(macAddr_v dstAddr, egressSpec_v port) {
        standard_metadata.egress_spec = port;
        // hdr.ethernet.srcAddr = hdr.ethernet.dstAddr; // TODO: It should be corrected further
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    table ipv4_lpm {
        key = {
            hdr.ipv4.dstAddr: lpm;
        }

        actions = {
            ipv4_forward;
            drop;
            NoAction;
        }

        size = 1024;
        default_action = NoAction();
    }
    
    apply {
        if (hdr.ipv4.isValid()) {
            ipv4_lpm.apply();
        }
    }
}

control MyEgress(inout headers_t hdr,
                 inout metadata_t meta,
                 inout standard_metadata_t standard_metadata) {
    apply {}
}

control MyComputeChecksum(inout headers_t hdr, inout metadata_t meta) {

    apply {
        update_checksum(
            hdr.ipv4.isValid(),
            { hdr.ipv4.version,
              hdr.ipv4.ihl,
              hdr.ipv4.diffserv,
              hdr.ipv4.totalLen,
              hdr.ipv4.identification,
              hdr.ipv4.flags,
              hdr.ipv4.fragOffset,
              hdr.ipv4.ttl,
              hdr.ipv4.protocol,
              hdr.ipv4.srcAddr,
              hdr.ipv4.dstAddr },
            hdr.ipv4.hdrChecksum,
            HashAlgorithm.csum16
        );
    }
}

control MyDeparser(packet_out packet, in headers_t hdr) {
    apply {
        packet.emit(hdr.ethernet);
        packet.emit(hdr.ipv4);
    }
}

V1Switch(
    MyParser(),
    MyVerifyChecksum(),
    MyIngress(),
    MyEgress(),
    MyComputeChecksum(),
    MyDeparser()
) main;
