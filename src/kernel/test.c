#include <onix/types.h>
#include <onix/assert.h>
#include <onix/cpu.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/errno.h>
#include <onix/string.h>
#include <onix/net.h>

err_t sys_test()
{
    // // 发送测试数据包
    // pbuf_t *pbuf = pbuf_get();
    // netif_t *netif = netif_get();

    // int len = 1500;
    // memset(pbuf->eth->payload, 'A', len);
    
    // ip_addr_t addr;
    // assert(inet_aton("192.168.6.1", addr) == EOK);
    // arp_eth_output(netif,pbuf,addr,0x9000,len);
    // return EOK;
    ip_addr_t addr;
    assert(inet_aton("192.168.6.2",addr)==EOK);

    pbuf_t *pbuf = pbuf_get();
    netif_t *netif = netif_get(addr);

    ip_t *ip=pbuf->eth->ip;
    u16 len=128-sizeof(ip_t)-sizeof(eth_t);
    memset(ip->payload,'T',len);
    ip_output(netif,pbuf,addr,254,len);
}
