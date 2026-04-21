// ======================================================================
// \title  SX1303Gateway.cpp
// \author ninhdh4
// \brief  cpp file for SX1303Gateway funtions
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"
#include "fprime-sx1303/Components/SX1303Manager/SX1303Gateway.hpp"

extern "C" {
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/config.h"
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/loragw_hal.h"

#define GW_DEBUG 1

#define	EXIT_FAILURE	    1	/* Failing exit status.  */
#define	EXIT_SUCCESS	    0	/* Successful exit status.  */

#define GW_RADIO_TYPE           LGW_RADIO_TYPE_SX1250
#define GW_CLK_SRC              0
#define GW_FINE_TIMESTAMP       false
#define GW_RSSI_OFFSET          -215.4
#define GW_FREQ_HZ              921000000U //915000000U

#define GW_NB_PKT_MAX           64 /* max number of packets per fetch/send cycle */

struct ChanMultiSF_t {
    bool enable;
    U8   radio;
    I32  if_freq;
};

struct ChanLoraStd_t {
    bool enable;
    U8   radio;
    I32  if_freq;
    U32  bandwidth;
    U8   spread_factor;
    bool implicit_hdr;
    U8   implicit_payload_length;
    bool implicit_crc_en;
    U8   implicit_coderate;
};

struct ChanFSK_t {
    bool enable;
    U8   radio;
    I32  if_freq;
    U32  bandwidth;   // Hz
    U32  datarate;    // bps
};

struct GwChPlanConfig_t {
    U32             radio0_freq;
    U32             radio1_freq;
    U8              sf_enable;
    ChanMultiSF_t   chan_multi_sf[8];   // chan_multiSF_0 .. 7
    ChanLoraStd_t   chan_lora_std;      // chan_Lora_std
    ChanFSK_t       chan_fsk;           // chan_FSK
};

static constexpr GwChPlanConfig_t CH_PLAN[] = {
    // [0] unused
    {},

    // [1] EU868
    {
        .radio0_freq    = 867500000,
        .radio1_freq    = 868500000,
        .sf_enable      = 0b11111100,  // SF7(bit2)~SF12(bit7) = 0xFC
        .chan_multi_sf  = {
            {true, 1, -400000},  // chan_multiSF_0: 868.5 - 0.4 = 868.1 MHz
            {true, 1, -200000},  // chan_multiSF_1: 868.5 - 0.2 = 868.3 MHz
            {true, 1,       0},  // chan_multiSF_2: 868.5 + 0.0 = 868.5 MHz
            {true, 0, -400000},  // chan_multiSF_3: 867.5 - 0.4 = 867.1 MHz
            {true, 0, -200000},  // chan_multiSF_4: 867.5 - 0.2 = 867.3 MHz
            {true, 0,       0},  // chan_multiSF_5: 867.5 + 0.0 = 867.5 MHz
            {true, 0,  200000},  // chan_multiSF_6: 867.5 + 0.2 = 867.7 MHz
            {true, 0,  400000},  // chan_multiSF_7: 867.5 + 0.4 = 867.9 MHz
        },
        .chan_lora_std  = {true, 0, -200000, 250000, 7, false, 17, false, 1},
        .chan_fsk       = {false, 1, 300000, 125000, 50000},
    },

    // [2] US915
    {
        .radio0_freq    = 902700000,
        .radio1_freq    = 903400000,
        .sf_enable      = 0b11111111,  // SF5~SF12
        .chan_multi_sf  = {
            {true, 0, -400000}, // 902.3 MHz
            {true, 0, -200000}, // 902.5 MHz
            {true, 0,       0}, // 902.7 MHz
            {true, 0,  200000}, // 902.9 MHz
            {true, 1, -300000}, // 903.1 MHz
            {true, 1, -100000}, // 903.3 MHz
            {true, 1,  100000}, // 903.5 MHz
            {true, 1,  300000}, // 903.7 MHz
        },
        .chan_lora_std  = {true, 0, 300000, 500000, 8, false, 17, false, 1},
        .chan_fsk       = {false, 1, 300000, 125000, 50000},
    },

    // [3] CN779
    {},
    // [4] EU433
    {},
    // [5] AU915
    {},
    // [6] CN470
    {},
    // [7] AS923
    {},

    // [8] AS923_2 // Vietnam
    {
        .radio0_freq    = 921500000,
        .radio1_freq    = 922300000,
        .sf_enable      = 0b11111100,  // SF7~SF12
        .chan_multi_sf  = {
            {true, 0, -300000}, // 921.2 MHz
            {true, 0, -100000}, // 921.4 MHz
            {true, 0,  100000}, // 921.6 MHz
            {true, 0,  300000}, // 921.8 MHz
            {true, 1, -300000}, // 922.0 MHz
            {true, 1, -100000}, // 922.2 MHz
            {true, 1,  100000}, // 922.4 MHz
            {true, 1,  300000}, // 922.6 MHz
        },
        .chan_lora_std  = {true, 0, 300000, 500000, 8, false, 17, false, 1},
        .chan_fsk       = {false, 1, 300000, 125000, 50000},
    },

    // [9] AS923_3
    {},
    // [10] KR920
    {},
    // [11] IN865
    {},
    // [12] RU864
    {},
    // [13] AS923_4
    {},
};

int8_t sx1303_config(bool publicNet, uint8_t region){
    struct lgw_conf_board_s boardconf;
    struct lgw_conf_ftime_s tsconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_conf_rxif_s ifconf;
    struct lgw_conf_demod_s demodconf;

    memset( &boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = publicNet;
    boardconf.clksrc = GW_CLK_SRC;
    boardconf.full_duplex = false;
    boardconf.com_type = LGW_COM_SPI;
    
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure board");
        return EXIT_FAILURE;
    }

    /* set timestamp configuration */
    tsconf.enable = GW_FINE_TIMESTAMP;
    tsconf.mode = LGW_FTIME_MODE_ALL_SF;
    if (GW_FINE_TIMESTAMP) {
        if (lgw_ftime_setconf(&tsconf) != LGW_HAL_SUCCESS) {
            sx1303_log_debug("ERROR: Failed to configure fine timestamp");
            return EXIT_FAILURE;
        }
    }
    else {
        sx1303_log_debug("INFO: Configuring legacy timestamp");
    }

    const GwChPlanConfig_t& gwcfg = CH_PLAN[region];

    if (gwcfg.radio0_freq == 0){
        sx1303_log_debug("ERROR: Not supported region: %d", region);
        return EXIT_FAILURE;
    }

    /* set configuration for RF chains */
    for (uint8_t radioidx = 0; radioidx < LGW_RF_CHAIN_NB; ++radioidx) {
        memset(&rfconf, 0, sizeof rfconf);
        rfconf.enable = true;
        rfconf.single_input_mode = false;
        rfconf.type = GW_RADIO_TYPE;
        rfconf.rssi_offset = GW_RSSI_OFFSET;

        /* enable TX on radio 0 */
        if (radioidx == 0) {
            rfconf.tx_enable = true;
            rfconf.freq_hz = gwcfg.radio0_freq;
        }
        else {
            rfconf.tx_enable = false;
            rfconf.freq_hz = gwcfg.radio1_freq;
        }

        rfconf.rssi_tcomp = { 0, 0, 20.41, 2162.56, 0 };

        sx1303_log_debug("INFO: radio %i enabled, center frequency %u, RSSI offset %d, tx enabled %d",
                         radioidx, rfconf.freq_hz, (int)rfconf.rssi_offset, rfconf.tx_enable);
        
        if (lgw_rxrf_setconf(radioidx, &rfconf) != LGW_HAL_SUCCESS) {
            sx1303_log_debug("ERROR: invalid configuration for radio %i", radioidx);
            return EXIT_FAILURE;
        }
    }

    /* set configuration for demodulators */
    memset(&demodconf, 0, sizeof demodconf);
    demodconf.multisf_datarate = gwcfg.sf_enable;
    if (lgw_demod_setconf(&demodconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: invalid configuration for demodulation parameters");
        return EXIT_FAILURE;
    }

    /* set configuration for Lora multi-SF channels (bandwidth cannot be set) */
    for (uint8_t ifidx = 0; ifidx < LGW_MULTI_NB; ++ifidx) {
        memset(&ifconf, 0, sizeof ifconf);

        const ChanMultiSF_t& chancfg = gwcfg.chan_multi_sf[ifidx];
        ifconf.enable = chancfg.enable;
        ifconf.rf_chain = chancfg.radio;
        ifconf.freq_hz = chancfg.if_freq;

        if (lgw_rxif_setconf(ifidx, &ifconf) != LGW_HAL_SUCCESS) {
            sx1303_log_debug("ERROR: failed to configure rxif %d", ifidx);
            return EXIT_FAILURE;
        }
    }

    /* set configuration for Lora standard channel */
    memset(&ifconf, 0, sizeof ifconf);
    ifconf.enable = gwcfg.chan_lora_std.enable;
    ifconf.rf_chain = gwcfg.chan_lora_std.radio;
    ifconf.freq_hz = gwcfg.chan_lora_std.if_freq;

    switch(gwcfg.chan_lora_std.bandwidth) {
        case 500000: ifconf.bandwidth = BW_500KHZ; break;
        case 250000: ifconf.bandwidth = BW_250KHZ; break;
        case 125000: ifconf.bandwidth = BW_125KHZ; break;
        default: ifconf.bandwidth = BW_UNDEFINED;
    }

    switch(gwcfg.chan_lora_std.spread_factor) {
        case  5: ifconf.datarate = DR_LORA_SF5;  break;
        case  6: ifconf.datarate = DR_LORA_SF6;  break;
        case  7: ifconf.datarate = DR_LORA_SF7;  break;
        case  8: ifconf.datarate = DR_LORA_SF8;  break;
        case  9: ifconf.datarate = DR_LORA_SF9;  break;
        case 10: ifconf.datarate = DR_LORA_SF10; break;
        case 11: ifconf.datarate = DR_LORA_SF11; break;
        case 12: ifconf.datarate = DR_LORA_SF12; break;
        default: ifconf.datarate = DR_UNDEFINED;
    }

    ifconf.implicit_hdr = gwcfg.chan_lora_std.implicit_hdr;
    if (ifconf.implicit_hdr == true) {
        ifconf.implicit_payload_length = gwcfg.chan_lora_std.implicit_payload_length;
        ifconf.implicit_crc_en = gwcfg.chan_lora_std.implicit_crc_en;
        ifconf.implicit_coderate = gwcfg.chan_lora_std.implicit_coderate;
    }
    if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: invalid configuration for Lora standard channel");
        return EXIT_FAILURE;
    }

    /* set configuration for FSK channel */
    memset(&ifconf, 0, sizeof ifconf);
    ifconf.enable = gwcfg.chan_fsk.enable;
    if (gwcfg.chan_fsk.enable) {
        ifconf.rf_chain = gwcfg.chan_fsk.radio;
        ifconf.datarate = gwcfg.chan_fsk.datarate;

        if (gwcfg.chan_fsk.bandwidth <= 125000) ifconf.bandwidth = BW_125KHZ;
        else if (gwcfg.chan_fsk.bandwidth <= 250000) ifconf.bandwidth = BW_250KHZ;
        else if (gwcfg.chan_fsk.bandwidth <= 500000) ifconf.bandwidth = BW_500KHZ;
        else ifconf.bandwidth = BW_UNDEFINED;
    }
    if (lgw_rxif_setconf(9, &ifconf) != LGW_HAL_SUCCESS) {
        sx1303_log_debug("ERROR: invalid configuration for FSK channel");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int8_t gateway_config(void) {

    /*
    "gateway_conf": {
        "gateway_ID": "AA555A0000000000",
        "server_address": "localhost",
        "serv_port_up": 1730,
        "serv_port_down": 1730,
        "keepalive_interval": 10,
        "stat_interval": 30,
        "push_timeout_ms": 100,
        "forward_crc_valid": true,
        "forward_crc_error": false,
        "forward_crc_disabled": false,
        "gps_tty_path": "/dev/ttyS0",
        "ref_latitude": 0.0,
        "ref_longitude": 0.0,
        "ref_altitude": 0,
        "beacon_period": 0,
        "beacon_freq_hz": 869525000,
        "beacon_datarate": 9,
        "beacon_bw_hz": 125000,
        "beacon_power": 14,
        "beacon_infodesc": 0
    }
    */

    return EXIT_SUCCESS;
}

int8_t sx1303_gateway_start(bool publicNet, uint8_t region){
    sx1303_log_debug("SX1302 HAL version ***\n%s\n***\n", lgw_version_info());
    sx1303_log_debug("Gateway Starting, %s Network, Channel Plan ID: %d, ", publicNet ? "Public" : "Private", region);
    
    if (sx1303_config(publicNet, region) != EXIT_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure board\n");
        return EXIT_FAILURE;
    }
    if (gateway_config() != EXIT_SUCCESS) {
        sx1303_log_debug("ERROR: failed to configure gateway\n");
        return EXIT_FAILURE;
    }

    sx1303_power_on(true);
    /* Board reset */
    sx1303_reset();

    /* connect, configure and start the LoRa concentrator */
    if (lgw_start() != 0) {
        sx1303_log_debug("ERROR: failed to start the gateway\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int8_t LoRaMacParserData( LoRaMacMessageData_t* macMsg ) {
    if( ( macMsg == 0 ) || ( macMsg->Buffer == 0 ) ) {
        return EXIT_FAILURE;
    }

    uint16_t bufItr = 0;

    macMsg->MHDR.Value = macMsg->Buffer[bufItr++];

    macMsg->FHDR.DevAddr = macMsg->Buffer[bufItr++];
    macMsg->FHDR.DevAddr |= ( ( uint32_t ) macMsg->Buffer[bufItr++] << 8 );
    macMsg->FHDR.DevAddr |= ( ( uint32_t ) macMsg->Buffer[bufItr++] << 16 );
    macMsg->FHDR.DevAddr |= ( ( uint32_t ) macMsg->Buffer[bufItr++] << 24 );

    macMsg->FHDR.FCtrl.Value = macMsg->Buffer[bufItr++];

    macMsg->FHDR.FCnt = macMsg->Buffer[bufItr++];
    macMsg->FHDR.FCnt |= macMsg->Buffer[bufItr++] << 8;

    /* 
    memcpy1( macMsg->FHDR.FOpts, &macMsg->Buffer[bufItr], macMsg->FHDR.FCtrl.Bits.FOptsLen );
    bufItr = bufItr + macMsg->FHDR.FCtrl.Bits.FOptsLen;

    // Initialize anyway with zero.
    macMsg->FPort = 0;
    macMsg->FRMPayloadSize = 0;

    if( ( macMsg->BufSize - bufItr - LORAMAC_MIC_FIELD_SIZE ) > 0 )
    {
        macMsg->FPort = macMsg->Buffer[bufItr++];

        macMsg->FRMPayloadSize = ( macMsg->BufSize - bufItr - LORAMAC_MIC_FIELD_SIZE );
        memcpy1( macMsg->FRMPayload, &macMsg->Buffer[bufItr], macMsg->FRMPayloadSize );
        bufItr = bufItr + macMsg->FRMPayloadSize;
    }
    */

    macMsg->MIC  = ( uint32_t )   macMsg->Buffer[( macMsg->BufSize - LORAMAC_MIC_FIELD_SIZE )];
    macMsg->MIC |= ( ( uint32_t ) macMsg->Buffer[( macMsg->BufSize - LORAMAC_MIC_FIELD_SIZE ) + 1] << 8 );
    macMsg->MIC |= ( ( uint32_t ) macMsg->Buffer[( macMsg->BufSize - LORAMAC_MIC_FIELD_SIZE ) + 2] << 16 );
    macMsg->MIC |= ( ( uint32_t ) macMsg->Buffer[( macMsg->BufSize - LORAMAC_MIC_FIELD_SIZE ) + 3] << 24 );

    return EXIT_SUCCESS;
}

int8_t DetermineFrameType( LoRaMacMessageData_t* macMsg, FType_t* fType ) {
    if( ( macMsg == NULL ) || ( fType == NULL ) ) {
        return EXIT_FAILURE;
    }

    /* The LoRaWAN specification allows several possible configurations how data up/down frames are built up.
     * In sake of clearness the following naming is applied. Please keep in mind that this is
     * implementation specific since there is no definition in the LoRaWAN specification included.
     *
     * X -> Field is available
     * - -> Field is not available
     *
     * +-------+  +----------+------+-------+--------------+
     * | FType |  | FOptsLen | Fopt | FPort |  FRMPayload  |
     * +-------+  +----------+------+-------+--------------+
     * |   A   |  |    > 0   |   X  |  > 0  |       X      |
     * +-------+  +----------+------+-------+--------------+
     * |   B   |  |   >= 0   |  X/- |   -   |       -      |
     * +-------+  +----------+------+-------+--------------+
     * |   C   |  |    = 0   |   -  |  = 0  | MAC commands |
     * +-------+  +----------+------+-------+--------------+
     * |   D   |  |    = 0   |   -  |  > 0  |       X      |
     * +-------+  +----------+------+-------+--------------+
     */

    if( ( macMsg->FHDR.FCtrl.Bits.FOptsLen > 0 ) && ( macMsg->FPort > 0 ) ) {
        *fType = FRAME_TYPE_A;
    }
    else if( macMsg->FRMPayloadSize == 0 ) {
        *fType = FRAME_TYPE_B;
    }
    else if( ( macMsg->FHDR.FCtrl.Bits.FOptsLen == 0 ) && ( macMsg->FPort == 0 ) ) {
        *fType = FRAME_TYPE_C;
    }
    else if( ( macMsg->FHDR.FCtrl.Bits.FOptsLen == 0 ) && ( macMsg->FPort > 0 ) ) {
        *fType = FRAME_TYPE_D;
    }
    else {
        // Should never happen.
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int8_t sx1303_gateway_receive(void){
    struct lgw_pkt_rx_s rxpkt[GW_NB_PKT_MAX];
    int8_t nb_pkt = lgw_receive(GW_NB_PKT_MAX, rxpkt);
    if (nb_pkt <= 0) {
        return EXIT_FAILURE;
    }

    Fw::Time currentTime;
    if (!sx1303_get_time(currentTime)){
        sx1303_log_debug("Get system time Error");
    }
    LoRaMacHeader_t macHdr;
    uint8_t pktHeaderLen = 0;

    for (int8_t i = 0; i < nb_pkt; i++) {
        if (rxpkt[i].status != STAT_CRC_OK) {
            continue; 
        }

        pktHeaderLen = 0;
        macHdr.Value = rxpkt[i].payload[pktHeaderLen++];

        // Accept frames of LoRaWAN Major Version 1 only
        if( macHdr.Bits.Major != 0 ) {
            sx1303_log_debug("Wrong Major version: %d", macHdr.Bits.Major);
            continue;
        }

        if (macHdr.Bits.MType != FRAME_TYPE_DATA_CONFIRMED_UP &&
            macHdr.Bits.MType != FRAME_TYPE_DATA_UNCONFIRMED_UP &&
            macHdr.Bits.MType != FRAME_TYPE_PROPRIETARY ){
            sx1303_log_debug("Skip unsupported message type: %d", macHdr.Bits.MType);
            continue;
        }

        LoRaMacMessageData_t macMsgData = {
            .Buffer = rxpkt[i].payload,
            .BufSize = rxpkt[i].size,
        };
        LoRaMacParserData(&macMsgData);
        sx1303_log_debug("DevAddr: 0x%08X", macMsgData.FHDR.DevAddr);
        FType_t fType;
        if( DetermineFrameType( &macMsgData, &fType ) != EXIT_SUCCESS ) {
            sx1303_log_debug("DetermineFrameType Error");
            continue;
        }

#if GW_DEBUG
        sx1303_log_debug("----- %s packet -----  size: %u, datarate %d, freq_hz %u, rssi_sig %d, ftime: %d",
                            (rxpkt[i].modulation == MOD_LORA) ? "LoRa" : "FSK",
                            rxpkt[i].size, rxpkt[i].datarate, rxpkt[i].freq_hz, (int)rxpkt[i].rssis, rxpkt[i].ftime);
        char hex_buf[rxpkt[i].size * 3 + 1];
        int offset = 0;
        for (uint8_t j = 0; j < rxpkt[i].size; j++) {
            offset += snprintf(hex_buf + offset, sizeof(hex_buf) - offset, "%02X ", rxpkt[i].payload[j]);
        }
        sx1303_log_debug("Payload: %s", hex_buf);
#endif

#if LORA_MAC_EXT
        sx1303_lora_out((const uint8_t *)& rxpkt[i], sizeof(rxpkt[i]));
#endif
        U64 epochMicro = (static_cast<U64>(currentTime.getSeconds()) * 1000000) + currentTime.getUSeconds();

        SX1303::SX1303Data data;
        data.set_time_unix(epochMicro);
        data.set_freq_hz(rxpkt[i].freq_hz);
        data.set_freq_offset(rxpkt[i].freq_offset);
        data.set_if_chain(rxpkt[i].if_chain);
        data.set_status(rxpkt[i].status);
        data.set_count_us(rxpkt[i].count_us);
        data.set_rf_chain(rxpkt[i].rf_chain);
        data.set_modem_id(rxpkt[i].modem_id);
        data.set_modulation(rxpkt[i].modulation);
        data.set_bandwidth(rxpkt[i].bandwidth);
        data.set_datarate(rxpkt[i].datarate);
        data.set_coderate(rxpkt[i].coderate);
        data.set_rssic(rxpkt[i].rssic);
        data.set_rssis(rxpkt[i].rssis);
        data.set_snr(rxpkt[i].snr);
        data.set_snr_min(rxpkt[i].snr_min);
        data.set_snr_max(rxpkt[i].snr_max);
        data.set_crc(rxpkt[i].crc);
        data.set_length(rxpkt[i].size);
        data.set_ftime_received(rxpkt[i].ftime_received);
        data.set_ftime(rxpkt[i].ftime);

        // Copy payload
        SX1303::LgwPayload pl;
        for (U16 j = 0; j < 256; j++) {
            pl[j] = rxpkt[i].payload[j];
        }
        data.set_payload(pl);

        sx1303_send_gw_packet(data);

    }

    return EXIT_SUCCESS;
}

} // extern "C"