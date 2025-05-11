#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

char LICENSE[] SEC("license") = "GPL";

struct __attribute__((packed)) int_option {
    __u8 Kind : 8;
    __u8 Length : 8;
    __u8 TagFreq : 4;
    __u8 LinkSpd : 4;
    __u8 INTval : 8;
    __u8 HopID : 8;
    __u32 HopLat : 24;
    __u8 INTEcr : 8;
    __u8 LnkSEcr : 4;
    __u8 HIDEcr : 4;
    __u16 HopLatEcr : 16;
};

struct __attribute__((packed)) key_t {
    __u32 src_ip;
    __u16 src_port;
};

// Define BPF map
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1024);
    __type(key, struct key_t);
    __type(value, struct int_option);

} int_option_map SEC(".maps");


SEC("xdp")  // Fixed section name
int xdp_tcp_parser(struct xdp_md *ctx) {  // Fixed function signature
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_DROP;

    // Only process IPv4 packets
    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return XDP_PASS;

    struct iphdr *ip = (struct iphdr *)(eth + 1);
    if ((void *)(ip + 1) > data_end)
        return XDP_DROP;

    bpf_printk("Received IP on enp7s0\n");
    // Only process TCP packets (protocol number 6)
    if (ip->protocol != 6)  // IPPROTO_TCP = 6
        return XDP_PASS;

    struct tcphdr *tcp = (struct tcphdr *)((__u8 *)ip + (ip->ihl * 4));
    if ((void *)(tcp + 1) > data_end)
        return XDP_DROP;

    if (tcp->doff == 5)
	return XDP_PASS;

    __u8 tcp_header_len = tcp->doff * 4;
    if (tcp_header_len < sizeof(struct tcphdr))
        return XDP_DROP;

    bpf_printk("TCP Packet: Src Port: %d, Dst Port: %d\n",
               bpf_ntohs(tcp->source), bpf_ntohs(tcp->dest));

    if (tcp->doff == 5)
        return XDP_PASS;

    __u8 *opts = (__u8 *)tcp + 20;

    // We assume that TCP-INT option is set as the first option. Otherwise we ignore it
    
    if (opts + 1 > (__u8 *)data_end || opts < (__u8*)data) // Drop if no kind is set
        return XDP_DROP;

    __u8 kind = *opts;
    if (kind != 0x72)
        return XDP_PASS;

    if (opts + 2 > (__u8 *)data_end || opts + 1 < (__u8*)data) // Drop if no length is set
        return XDP_DROP;

    __u8 length = *(opts + 1);
    if ((opts + length > (__u8 *)data_end) || (opts + length - 1 < (__u8*)data)) // Drop if data is not set properly
        return XDP_DROP;

    struct int_option *option = (struct int_option *)(opts);
    if (((void *)(option + 1) > data_end) || ((void *)option < data))
        return XDP_DROP;
    
    // TODO: Switch value of TagFreq w. LinkSpd and LnkSEcr w. HIDEcr due to Endianess


    // bpf_printk("TCP-OPT kind: %x length: %d\n", option->Kind, option->Length);
    // bpf_printk("TCP-OPT TagFreq: %x \n", option->TagFreq);
    // bpf_printk("TCP-OPT LinkSpd: %x \n", option->LinkSpd);
    // bpf_printk("TCP-OPT INTval: %x \n", option->INTval);
    // bpf_printk("TCP-OPT HopID: %x \n", option->HopID);
    // bpf_printk("TCP-OPT HopLat: %x \n", option->HopLat);
    // bpf_printk("TCP-OPT INTEcr: %x \n", option->INTEcr);
    // bpf_printk("TCP-OPT LnkSEcr: %x \n", option->LnkSEcr);
    // bpf_printk("TCP-OPT HIDEcr: %x \n", option->HIDEcr);
    // bpf_printk("TCP-OPT HopLatEcr: %x \n", option->HopLatEcr);
 
    // Parse All TCP Options:
    // - Not Functional
    // - Verifier Rejects
    // - Check out https://github.com/gamemann/XDP-TCP-Header-Options/tree/master and https://netdevconf.info//0x14/pub/slides/50/Issuing%20SYN%20Cookies%20in%20XDP.pdf
    //
    //
    // __u16 optdata = 0;
    // while (optdata <= 40) {
    //     __u8 *kind = opts + optdata;
    //     
    //     if (kind + 1 > (__u8 *)data_end || kind < (__u8 *)data)
    //         return XDP_DROP;
    //     
    //     if (*kind == 0) 
    //     {
    //         bpf_printk("TCP-OPT kind: %x\n", *kind);
    //         optdata++;
    //         break;
    //     }
    //    	else if (*kind == 1)
    //     {
    //         bpf_printk("TCP-OPT kind: %x\n", *kind);
    //         optdata++;
    //         continue;
    //     }
    //     else 
    //     {
    //         __u8 *len = kind + 1;
    //         if (len + 1 > (__u8 *)data_end || len < (__u8 *)data) // check for validity of length
    //             return XDP_DROP;

    //         if (kind + *len > (__u8 *)data_end)
    //     	return XDP_DROP;
    //         
    //         optdata += *len > 0 ? *len: 1;
    //         bpf_printk("TCP-OPT kind: %x length: %d\n", *kind, len);
    //     }

    //     // not required but insereted to avoid infinite loops
    //     optdata++;
    // }
    

    struct key_t key = {
    	.src_ip = bpf_ntohl(ip->saddr),
	.src_port = bpf_ntohs(tcp->source)
    };

    bpf_map_update_elem(&int_option_map, &key, option, BPF_ANY);
    return XDP_PASS;
}