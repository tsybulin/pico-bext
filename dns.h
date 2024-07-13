#pragma once

#include "lwipopts.h"

#include "lwip/err.h"
#include "lwip/ip_addr.h"

err_t best_dns_gethostbyname(const char *hostname, ip_addr_t *addr) ;
