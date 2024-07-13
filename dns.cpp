#include "dns.h"

#include "pico/cyw43_arch.h"
#include "lwip/dns.h"

#include "lwipopts.h"

typedef struct dns_t_ {
    ip_addr_t addr ;
    bool ready ;
    bool found ;
} dns_t ;

void best_dns_cb(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    dns_t *dt = (dns_t *) arg ;
    dt->ready = true ;

    if (!ipaddr) {
        return ;
    }

    dt->addr = *ipaddr ;
    dt->found = true ;
}

err_t best_dns_gethostbyname(const char *hostname, ip_addr_t *addr) {
    dns_t dt ;
    dt.found = false ;
    dt.ready = false ;

    cyw43_arch_lwip_begin() ;
    err_t err = dns_gethostbyname(hostname, addr, best_dns_cb, &dt) ;
    cyw43_arch_lwip_end() ;

    if (err != ERR_OK && err != ERR_INPROGRESS) {
        return err ;
    }

    absolute_time_t at = get_absolute_time() ;
    
    while (!dt.ready) {
        cyw43_arch_poll() ;
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000)) ;
        
        if (absolute_time_diff_us(at, get_absolute_time()) > 5000000L) {
            return ERR_ABRT ;
        }
    }

    if (!dt.found) {
        return ERR_VAL ;
    }

    *addr = dt.addr ;

    return ERR_OK ;
}
