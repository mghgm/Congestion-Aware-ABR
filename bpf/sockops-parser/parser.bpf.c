#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define INT_HDR_LEN 0x0c
#define INT_KIND 0x72
#define QUEUE_MAX_ENTRIES 4096

#define BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG 16
#define BPF_SOCK_OPS_PARSE_HDR_OPT_CB 13


struct __attribute__((packed)) int_option {
    __u64 Timestamp: 64;
    __u8  queue_depth: 8;
    __u16 timedelta: 16;
};

// Define BPF map
struct {
    __uint(type, BPF_MAP_TYPE_QUEUE);
    __uint(max_entries, QUEUE_MAX_ENTRIES);
    __uint(value_size, sizeof(parsed_opt));
    __uint(key_size, 0);

} int_map SEC(".maps");


SEC("sockops")
int bpf_sockops_parse_tcp_options(struct bpf_sock_ops *skops) {
    if (skops->op == BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB || skops->op == BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB) {
        // TODO: Clear up the map
        
        int rv;
        rv = bpf_sock_ops_cb_flags_set(skops, BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG);
        if (rv < 0) {
            bpf_printk("Failed to setup flag:: BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG, %d", rv);
            return 1;
        }
    }
    else if (skops->op == BPF_SOCK_OPS_PARSE_HDR_OPT_CB) {
	    char opt_buf[12] = {0x00, 0x00, 0x00, 0x00,
	    		            0x00, 0x00, 0x00, 0x00,
	    		            0x00, 0x00, 0x00, 0x00};
        opt_buf[0] = INT_KIND;
	    
	    int res;
	    res = bpf_load_hdr_opt(skops, (void *)opt_buf, INT_HDR_LEN, 0);
	    if (res > 0) {

    	    struct int_option parsed_opt = {0};
            parsed_opt.queue_depth = opt_buf[8];
            parsed_opt.timedelta = *((__u16 *) &opt_buf[11]);

            // TODO: Put data into parsed_option from buf
            
    	    bpf_printk("found! enq_depth = %d timestamp = %lld\n", opt_buf[8] , bpf_ktime_get_boot_ns());
            bpf_printk("parsed! queue_depth = %d timedelta = %d", parsed_opt.queue_depth, parsed_opt.timedelta);

            int rv;
            rv = bpf_map_push_elem(&int_map, &parsed_opt, 0);
            if (rv < 0) {
                bpf_printk("Failed to push to map: %d", rv);
                return 1;
            }
        }
    }

    return 1;
}

char _license[] SEC("license") = "GPL";

