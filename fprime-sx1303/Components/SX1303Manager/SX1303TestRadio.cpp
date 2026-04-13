// ======================================================================
// \title  SX1303TestRadio.cpp
// \author ninhdh4
// \brief  cpp file for SX1303TestRadio funtions
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"
#include "fprime-sx1303/Components/SX1303Manager/SX1303TestRadio.hpp"

extern "C" {
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/config.h"
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/loragw_hal.h"


#define	EXIT_FAILURE	    1	/* Failing exit status.  */
#define	EXIT_SUCCESS	    0	/* Successful exit status.  */

#define DEFAULT_CLK_SRC     0
#define DEFAULT_FREQ_HZ     921000000U //915000000U
#define COM_TYPE_DEFAULT    LGW_COM_SPI
#define COM_PATH_DEFAULT    "/dev/spidev0.0"

int sx1303_test_hal_tx(void) {
    int i, x;
    unsigned int ft = DEFAULT_FREQ_HZ;
    int8_t rf_power = 2;
    uint8_t sf = 0;
    uint16_t bw_khz = 125;
    unsigned int nb_pkt = 1;
    unsigned int nb_loop = 1, cnt_loop;
    uint8_t size = 0;
    char mod[64] = "LORA";
    float br_kbps = 50;
    uint8_t fdev_khz = 25;
    int8_t freq_offset = 0;
    double arg_d = 0.0;
    unsigned int arg_u;
    int arg_i;
    char arg_s[64];
    float xf = 0.0;
    uint8_t clocksource = 0;
    uint8_t rf_chain = 0;
    lgw_radio_type_t radio_type = LGW_RADIO_TYPE_SX1250;
    uint16_t preamble = 8;
    bool invert_pol = false;
    bool no_header = false;
    bool single_input_mode = false;
    bool full_duplex = false;

    struct lgw_conf_board_s boardconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_pkt_tx_s pkt;
    struct lgw_tx_gain_lut_s txlut; /* TX gain table */
    uint8_t tx_status;
    uint32_t count_us;
    uint32_t trig_delay_us = 1000000;
    bool trig_delay = false;

    /* SPI interfaces */
    const char com_path_default[] = COM_PATH_DEFAULT;
    const char * com_path = com_path_default;
    lgw_com_type_t com_type = COM_TYPE_DEFAULT;

    /* Initialize TX gain LUT */
    txlut.size = 0;
    memset(txlut.lut, 0, sizeof txlut.lut);

    /* Summary of packet parameters */
    if (strcmp(mod, "CW") == 0) {
        sx1303_log_debug("Sending %i CW on %u Hz (Freq. offset %d kHz) at %i dBm\n", nb_pkt, ft, freq_offset, rf_power);
    }
    else if (strcmp(mod, "FSK") == 0) {
        sx1303_log_debug("Sending %i FSK packets on %u Hz (FDev %u kHz, Bitrate %.2f, %i bytes payload, %i symbols preamble) at %i dBm\n", nb_pkt, ft, fdev_khz, br_kbps, size, preamble, rf_power);
    } else {
        sx1303_log_debug("Sending %i LoRa packets on %u Hz (BW %i kHz, SF %i, CR %i, %i bytes payload, %i symbols preamble, %s header, %s polarity) at %i dBm\n", nb_pkt, ft, bw_khz, sf, 1, size, preamble, (no_header == false) ? "explicit" : "implicit", (invert_pol == false) ? "non-inverted" : "inverted", rf_power);
    }

    /* Configure the gateway */
    memset( &boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = true;
    boardconf.clksrc = clocksource;
    boardconf.full_duplex = full_duplex;
    boardconf.com_type = com_type;
    strncpy(boardconf.com_path, com_path, sizeof boardconf.com_path);
    boardconf.com_path[sizeof boardconf.com_path - 1] = '\0'; /* ensure string termination */
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure board\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true; /* rf chain 0 needs to be enabled for calibration to work on sx1257 */
    rfconf.freq_hz = ft;
    rfconf.type = radio_type;
    rfconf.tx_enable = true;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure rxrf 0\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = (((rf_chain == 1) || (clocksource == 1)) ? true : false);
    rfconf.freq_hz = ft;
    rfconf.type = radio_type;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure rxrf 1\n");
        return EXIT_FAILURE;
    }

    if (txlut.size > 0) {
        if (lgw_txgain_setconf(rf_chain, &txlut) != LGW_HAL_SUCCESS) {
            sx1303_log_debug("ERROR: failed to configure txgain lut\n");
            return EXIT_FAILURE;
        }
    }

    if (com_type == LGW_COM_SPI) {
        sx1303_power_on(true);
        /* Board reset */
        sx1303_reset();
    }

    /* connect, configure and start the LoRa concentrator */
    x = lgw_start();
    if (x != 0) {
        sx1303_log_debug("ERROR: %s: failed to start the gateway\n", __func__);
        return EXIT_FAILURE;
    }

    /* Send packets */
    memset(&pkt, 0, sizeof pkt);
    pkt.rf_chain = rf_chain;
    pkt.freq_hz = ft;
    pkt.rf_power = rf_power;
    if (trig_delay == false) {
        pkt.tx_mode = IMMEDIATE;
    } else {
        if (trig_delay_us == 0) {
            pkt.tx_mode = ON_GPS;
        } else {
            pkt.tx_mode = TIMESTAMPED;
        }
    }
    if ( strcmp( mod, "CW" ) == 0 ) {
        pkt.modulation = MOD_CW;
        pkt.freq_offset = freq_offset;
        pkt.f_dev = fdev_khz;
    }
    else if( strcmp( mod, "FSK" ) == 0 ) {
        pkt.modulation = MOD_FSK;
        pkt.no_crc = false;
        pkt.datarate = br_kbps * 1e3;
        pkt.f_dev = fdev_khz;
    } else {
        pkt.modulation = MOD_LORA;
        pkt.coderate = CR_LORA_4_5;
        pkt.no_crc = true;
    }
    pkt.invert_pol = invert_pol;
    pkt.preamble = preamble;
    pkt.no_header = no_header;
    pkt.payload[0] = 0x40; /* Confirmed Data Up */
    pkt.payload[1] = 0xAB;
    pkt.payload[2] = 0xAB;
    pkt.payload[3] = 0xAB;
    pkt.payload[4] = 0xAB;
    pkt.payload[5] = 0x00; /* FCTrl */
    pkt.payload[6] = 0;    /* FCnt */
    pkt.payload[7] = 0;    /* FCnt */
    pkt.payload[8] = 0x02; /* FPort */
    for (i = 9; i < 255; i++) {
        pkt.payload[i] = i;
    }

    for (i = 0; i < (int)nb_pkt; i++) {
        if (trig_delay == true) {
            if (trig_delay_us > 0) {
                lgw_get_instcnt(&count_us);
                sx1303_log_debug("count_us:%u\n", count_us);
                pkt.count_us = count_us + trig_delay_us;
                sx1303_log_debug("programming TX for %u\n", pkt.count_us);
            } else {
                sx1303_log_debug("programming TX for next PPS (GPS)\n");
            }
        }

        if( strcmp( mod, "LORA" ) == 0 ) {
            pkt.datarate = 7;
        }

        pkt.bandwidth = BW_125KHZ;
        pkt.size = 32;

        pkt.payload[6] = (uint8_t)(i >> 0); /* FCnt */
        pkt.payload[7] = (uint8_t)(i >> 8); /* FCnt */
        x = lgw_send(&pkt);
        if (x != 0) {
            sx1303_log_debug("ERROR: failed to send packet\n");
            break;
        }
        /* wait for packet to finish sending */
        do {
            sx1303_delay_ms(5);
            lgw_status(pkt.rf_chain, TX_STATUS, &tx_status); /* get TX status */
        } while (tx_status != TX_FREE);

        sx1303_log_debug("TX done\n");
    }

    sx1303_log_debug( "\nNb packets sent: %u (%u)\n", i, cnt_loop + 1 );

    /* Stop the gateway */
    x = lgw_stop();
    if (x != 0) {
        sx1303_log_debug("ERROR: failed to stop the gateway\n");
    }


    sx1303_log_debug("=========== Test End ===========\n");
    return 0;
}

int sx1303_test_hal_rx(void) {
    /* SPI interfaces */
    const char com_path_default[] = COM_PATH_DEFAULT;
    const char * com_path = com_path_default;
    lgw_com_type_t com_type = COM_TYPE_DEFAULT;

    int i, j, x;
    uint32_t fa = DEFAULT_FREQ_HZ;
    uint32_t fb = DEFAULT_FREQ_HZ + 1000000;
    double arg_d = 0.0;
    unsigned int arg_u;
    uint8_t clocksource = 0;
    lgw_radio_type_t radio_type = LGW_RADIO_TYPE_SX1250;
    uint8_t max_rx_pkt = 16;
    bool single_input_mode = false;
    float rssi_offset = -215.4;
    bool full_duplex = false;

    struct lgw_conf_board_s boardconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_conf_rxif_s ifconf;

    unsigned long nb_pkt_crc_ok = 0, nb_loop = 0, cnt_loop;
    int nb_pkt;

    uint8_t channel_mode = 0; /* LoRaWAN-like */

    const int32_t channel_if_mode0[9] = {
        -400000,
        -200000,
        0,
        -400000,
        -200000,
        0,
        200000,
        400000,
        -200000 /* lora service */
    };

    const int32_t channel_if_mode1[9] = {
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000 /* lora service */
    };

    const uint8_t channel_rfchain_mode0[9] = { 1, 1, 1, 0, 0, 0, 0, 0, 1 };

    const uint8_t channel_rfchain_mode1[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    sx1303_log_debug("===== sx1302 HAL RX test =====\n");

    /* Configure the gateway */
    memset( &boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = true;
    boardconf.clksrc = clocksource;
    boardconf.full_duplex = full_duplex;
    boardconf.com_type = com_type;
    strncpy(boardconf.com_path, com_path, sizeof boardconf.com_path);
    boardconf.com_path[sizeof boardconf.com_path - 1] = '\0'; /* ensure string termination */
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure board\n");
        return EXIT_FAILURE;
    }

    /* set configuration for RF chains */
    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fa;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure rxrf 0\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fb;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure rxrf 1\n");
        return EXIT_FAILURE;
    }

    /* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
    memset(&ifconf, 0, sizeof(ifconf));
    for (i = 0; i < 8; i++) {
        ifconf.enable = true;
        if (channel_mode == 0) {
            ifconf.rf_chain = channel_rfchain_mode0[i];
            ifconf.freq_hz = channel_if_mode0[i];
        } else if (channel_mode == 1) {
            ifconf.rf_chain = channel_rfchain_mode1[i];
            ifconf.freq_hz = channel_if_mode1[i];
        } else {
            sx1303_log_debug("ERROR: channel mode not supported\n");
            return EXIT_FAILURE;
        }
        ifconf.datarate = DR_LORA_SF7;
        if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
            sx1303_log_debug("ERROR: failed to configure rxif %d\n", i);
            return EXIT_FAILURE;
        }
    }

    /* set configuration for LoRa Service channel */
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.rf_chain = channel_rfchain_mode0[i];
    ifconf.freq_hz = channel_if_mode0[i];
    ifconf.datarate = DR_LORA_SF7;
    ifconf.bandwidth = BW_250KHZ;
    if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure rxif for LoRa service channel\n");
        return EXIT_FAILURE;
    }

    /* set the buffer size to hold received packets */
    struct lgw_pkt_rx_s rxpkt[max_rx_pkt];
    sx1303_log_debug("INFO: rxpkt buffer size is set to %u\n", max_rx_pkt);
    sx1303_log_debug("INFO: Select channel mode %u\n", channel_mode);

    /* Loop until user quits */
    cnt_loop = 0;
    while( true )
    {
        cnt_loop += 1;

        if (com_type == LGW_COM_SPI) {
            /* Board reset */
            sx1303_reset();
        }

        /* connect, configure and start the LoRa concentrator */
        x = lgw_start();
        if (x != 0) {
            sx1303_log_debug("ERROR: failed to start the gateway\n");
            return EXIT_FAILURE;
        }

        /* Loop until we have enough packets with CRC OK */
        sx1303_log_debug("Waiting for packets...\n");
        nb_pkt_crc_ok = 0;
        while (((nb_pkt_crc_ok < nb_loop) || nb_loop == 0)) {
            /* fetch N packets */
            nb_pkt = lgw_receive(sizeof(rxpkt), rxpkt);

            if (nb_pkt == 0) {
                sx1303_delay_ms(10);
            } else if (nb_pkt < 0) {
                sx1303_log_debug("ERROR: failed to receive packets\n");
                break;
            } else {
                for (i = 0; i < nb_pkt; i++) {
                    if (rxpkt[i].status == STAT_CRC_OK) {
                        nb_pkt_crc_ok += 1;
                    }
                    // sx1303_log_debug("\n----- %s packet -----\n", (rxpkt[i].modulation == MOD_LORA) ? "LoRa" : "FSK");
                    // sx1303_log_debug("  count_us: %u\n", rxpkt[i].count_us);
                    sx1303_log_debug("----- %s packet -----  size: %u, freq_hz %u, rssi_sig %.1f",
                                     (rxpkt[i].modulation == MOD_LORA) ? "LoRa" : "FSK",
                                     rxpkt[i].size, rxpkt[i].freq_hz, rxpkt[i].rssis);
                    // sx1303_log_debug("  chan:     %u\n", rxpkt[i].if_chain);
                    // sx1303_log_debug("  status:   0x%02X\n", rxpkt[i].status);
                    // sx1303_log_debug("  datr:     %u\n", rxpkt[i].datarate);
                    // sx1303_log_debug("  codr:     %u\n", rxpkt[i].coderate);
                    // sx1303_log_debug("  rf_chain  %u\n", rxpkt[i].rf_chain);
                    // sx1303_log_debug("  freq_hz %u", rxpkt[i].freq_hz);
                    // sx1303_log_debug("  snr_avg:  %.1f\n", rxpkt[i].snr);
                    // sx1303_log_debug("  rssi_chan:%.1f", rxpkt[i].rssic);
                    // sx1303_log_debug("  rssi_sig :%.1f", rxpkt[i].rssis);
                    // sx1303_log_debug("  crc:      0x%04X\n", rxpkt[i].crc);
                    // for (j = 0; j < rxpkt[i].size; j++) {
                    //     sx1303_log_debug("%02X ", rxpkt[i].payload[j]);
                    // }
                    // sx1303_log_debug("\n");
                    char hex_buf[rxpkt[i].size * 3 + 1];
                    int offset = 0;
                    for (j = 0; j < rxpkt[i].size; j++) {
                        offset += snprintf(hex_buf + offset, sizeof(hex_buf) - offset, "%02X ", rxpkt[i].payload[j]);
                    }
                    sx1303_log_debug("Payload: %s", hex_buf);
                    sx1303_lora_out(rxpkt[i].payload, rxpkt[i].size);
                }
                sx1303_log_debug("Received %d packets (total:%lu)\n", nb_pkt, nb_pkt_crc_ok);
            }
        }

        sx1303_log_debug( "\nNb valid packets received: %lu CRC OK (%lu)\n", nb_pkt_crc_ok, cnt_loop );

        /* Stop the gateway */
        x = lgw_stop();
        if (x != 0) {
            sx1303_log_debug("ERROR: failed to stop the gateway\n");
            return EXIT_FAILURE;
        }

        if (com_type == LGW_COM_SPI) {
            /* Board reset */
            sx1303_reset();
        }
    }

    sx1303_log_debug("=========== Test End ===========\n");

    return 0;
}

} // extern "C"