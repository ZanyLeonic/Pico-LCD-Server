/*
 * Copyright (c) 2021 Sandeep Mistry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"

#include "lwip/dhcp.h"
#include "lwip/init.h"

#include "lwip/apps/httpd.h"

#include "rmii_ethernet/netif.h"

#include "lcd_api.h"
#include "base64.h"

#include <string.h>

bool lightState = false;
bool lastState = false;

static const char *text_parser(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static const tCGI cgi_handlers[] = {
  {
    "/submit_text.cgi",
    text_parser
  }
};

char szBuf[32];

struct lcdapi_data lcd_api_cfg = {
    2,
    16,
    21, // rs pin
    19, // enable pin

    2, // Data pins
    3,
    4,
    5,

    0, // Cursors
    0,
};

void netif_link_callback(struct netif *netif)
{
    gpio_put(16, netif_is_link_up(netif));
}

void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    sleep_ms(5000);
    LCD_Clear(&lcd_api_cfg);

    snprintf(szBuf, sizeof(szBuf), "IP Address:\n%s", ip4addr_ntoa(netif_ip4_addr(netif)));

    LCD_PutStr(&lcd_api_cfg, szBuf);
    LCD_MoveTo(&lcd_api_cfg, 16, 0);

    gpio_put(25, 1);
}

int main() {

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    gpio_init(16);
    gpio_set_dir(16, GPIO_OUT);

    // LWIP network interface
    struct netif netif;

    //
    struct netif_rmii_ethernet_config netif_config = {
        pio0, // PIO:            0
        0,    // pio SM:         0 and 1
        6,    // rx pin start:   6, 7, 8    => RX0, RX1, CRS
        10,   // tx pin start:   10, 11, 12 => TX0, TX1, TX-EN
        14,   // mdio pin start: 14, 15   => ?MDIO, MDC
        NULL, // MAC address (optional - NULL generates one based on flash id) 
    };

    LCD_Init(&lcd_api_cfg);
    LCD_PutStr(&lcd_api_cfg, "Starting...\nv10");

    // change the system clock to use the RMII reference clock from pin 20
    clock_configure_gpin(clk_sys, 20, 50 * MHZ, 50 * MHZ);
    sleep_ms(100);

    // initialize stdio after the clock change
    stdio_init_all();
    sleep_ms(5000);

    // initilize LWIP in NO SYS mode
    lwip_init();

    // initialize the PIO base RMII Ethernet network interface
    netif_rmii_ethernet_init(&netif, &netif_config);
    
    // assign callbacks for link and status
    netif_set_link_callback(&netif, netif_link_callback);
    netif_set_status_callback(&netif, netif_status_callback);

    // set the default interface and bring it up
    netif_set_default(&netif);
    netif_set_up(&netif);

    // Start DHCP client and httpd
    dhcp_start(&netif);
    httpd_init();

    // setup core 1 to monitor the RMII ethernet interface
    // this let's core 0 do other things :)
    multicore_launch_core1(netif_rmii_ethernet_loop);

    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));

    while (1) {
        tight_loop_contents();
    }

    return 0;
}

static const char *
text_parser(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    char* fline = "";
    char* sline = "";

    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "fline"))
            fline = pcValue[i];
        else if (strcmp(pcParam[i], "sline"))
            sline = pcValue[i];
    }

    int fLineLen = StringSize(fline);
    int sLineLen = StringSize(sline);

    if (fLineLen < 4 || sLineLen < 4) return "/error.html";
    if (fLineLen > 24 || sLineLen > 24) return "/error.html";

    LCD_Clear(&lcd_api_cfg);

    unsigned char *out_a = malloc( b64d_size(fLineLen)+1 );
    unsigned char *out_b = malloc( b64d_size(sLineLen)+1 );

    int out_size_a = b64_decode(fline, fLineLen + 1, out_a);
    int out_size_b = b64_decode(sline, sLineLen + 1, out_b);

    out_a[out_size_a] = '\0';
    out_b[out_size_b] = '\0';

    snprintf(szBuf, sizeof(szBuf), "%s\n%s", out_a, out_b);

    LCD_PutStr(&lcd_api_cfg, szBuf);

    return "/success.html";
}