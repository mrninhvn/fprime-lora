module SX1303 {

    @ Payload buffer for a LoRa RX packet (always 256 bytes).
    @ Only the first 'length' bytes are valid data.
    array LgwPayload = [256] U8

    @ Struct representing SX1303 sensor data
    struct SX1303Data {
        @ Unix timestamp, in microseconds
        time_unix: U64

        @ GPS time, milliseconds since 06.Jan.1980
        time_gps: U64

        @ Central frequency of the IF chain, in Hz
        freq_hz: U32

        @ Frequency offset, in Hz (can be negative)
        freq_offset: I32

        @ Index of the IF chain that received the packet (0-based)
        if_chain: U8

        @ Status of the received packet
        @ (e.g. CRC OK, CRC bad, no CRC — see lgw_pkt_rx_status_e)
        status: U8

        @ Internal concentrator timestamp at time of reception,
        @ 1 microsecond resolution, wraps around at ~4295 seconds
        count_us: U32

        @ Index of the RF chain through which the packet was received
        rf_chain: U8

        @ Modem identifier used internally by the concentrator
        modem_id: U8

        @ Modulation scheme used by the packet
        @ (e.g. LORA = 0x10, FSK = 0x20 — see lgw_modulation_e)
        modulation: U8

        @ Modulation bandwidth in Hz (LoRa only)
        @ (e.g. BW_125KHZ, BW_250KHZ — see lgw_bandwidth_e)
        bandwidth: U8

        @ RX datarate — spreading factor for LoRa (e.g. 7–12),
        @ or bitrate in bps for FSK
        datarate: U32

        @ Forward error-correction code rate (LoRa only)
        @ (e.g. CR_4_5, CR_4_6 — see lgw_coderate_e)
        coderate: U8

        @ Average RSSI of the channel in dBm
        rssic: F32

        @ Average RSSI of the signal in dBm
        rssis: F32

        @ Average packet SNR in dB (LoRa only)
        snr: F32

        @ Minimum packet SNR in dB (LoRa only)
        snr_min: F32

        @ Maximum packet SNR in dB (LoRa only)
        snr_max: F32

        @ CRC value received in the packet payload
        crc: U16

        @ Actual payload size in bytes (0–255).
        @ Use this to determine how many bytes in payload are valid
        length: U16

        @ Raw payload buffer (always 256 bytes, only 'size' bytes are valid)
        payload: LgwPayload

        @ True if a fine timestamp was successfully received for this packet
        ftime_received: bool

        @ Fine timestamp: nanoseconds elapsed since the last PPS pulse.
        @ Only valid when ftime_received is true
        ftime: U32
    }
} 