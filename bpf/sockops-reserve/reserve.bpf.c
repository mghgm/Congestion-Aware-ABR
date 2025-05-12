#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#include <bpf/bpf_endian.h>

#define BPF_SOCK_OPS_WRITE_HDR_OPT_CB 15
#define BPF_SOCK_OPS_HDR_OPT_LEN_CB 14
#define BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG 64

#define DSCP (23 << 2)
#define IPPROTO_IP 0
#define IP_TOS 1


SEC("sockops")
int bpf_sockops_parse_tcp_options(struct bpf_sock_ops *skops) {
    if (skops->op == BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB || skops->op == BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB) {
        int rv;
        rv = bpf_sock_ops_cb_flags_set(skops, BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG);
        if (rv < 0) {
            bpf_printk("Failed to set flag:: BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG, %d", rv);
        }
        
        int rc;
        int dscp;
        dscp = DSCP;
        rc = bpf_setsockopt(skops, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp));
        if (rc < 0) {
            bpf_printk("Failed to set DSCP through:: bpf_setsockopt, %d", rc);
        }
    }
    else if (skops->op == BPF_SOCK_OPS_HDR_OPT_LEN_CB) {
        int rv;
        rv = bpf_reserve_hdr_opt(skops, 0x0c, 0);
        if (rv) {
            bpf_printk("Failed to reserve option, %d", rv);
        }
    }
    else if (skops->op == BPF_SOCK_OPS_WRITE_HDR_OPT_CB) {
        char opt[12] = {0x72, 0x0c, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00};
        int rv;
        rv = bpf_store_hdr_opt(skops, (void *)opt, 0x0c, 0);
        if (rv) {
            bpf_printk("Failed to store opt\n");
        }
    }

    return 1;
}

char _license[] SEC("license") = "GPL";

