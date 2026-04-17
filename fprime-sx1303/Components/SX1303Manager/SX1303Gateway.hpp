// ======================================================================
// \title  SX1303Gateway.hpp
// \author ninhdh4
// \brief  hpp file for SX1303Gateway funtions
// ======================================================================

#ifndef SX1303_SX1303Gateway_HPP
#define SX1303_SX1303Gateway_HPP

extern "C" {

/*! MAC header field size */
#define LORAMAC_MHDR_FIELD_SIZE             1

/*! ReJoinType field size */
#define LORAMAC_JOIN_TYPE_FIELD_SIZE        1

/*! Join EUI field size */
#define LORAMAC_JOIN_EUI_FIELD_SIZE         8

/*! Device EUI field size */
#define LORAMAC_DEV_EUI_FIELD_SIZE          8

/*! End-device nonce field size */
#define LORAMAC_DEV_NONCE_FIELD_SIZE        2

/*! Join-server nonce field size */
#define LORAMAC_JOIN_NONCE_FIELD_SIZE       3

/*! RJcount0 field size */
#define LORAMAC_RJCOUNT_0_FIELD_SIZE        2

/*! RJcount1 field size */
#define LORAMAC_RJCOUNT_1_FIELD_SIZE        2

/*! Network ID field size */
#define LORAMAC_NET_ID_FIELD_SIZE           3

/*! Device address field size */
#define LORAMAC_DEV_ADDR_FIELD_SIZE         4

/*! DLSettings field size */
#define LORAMAC_DL_SETTINGS_FIELD_SIZE      1

/*! RxDelay field size */
#define LORAMAC_RX_DELAY_FIELD_SIZE         1

/*! CFList field size */
#define LORAMAC_CF_LIST_FIELD_SIZE          16

/*! FHDR Device address field size */
#define LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE    LORAMAC_DEV_ADDR_FIELD_SIZE

/*! FHDR Frame control field size */
#define LORAMAC_FHDR_F_CTRL_FIELD_SIZE      1

/*! FHDR Frame control field size */
#define LORAMAC_FHDR_F_CNT_FIELD_SIZE       2

/*! FOpts maximum field size */
#define LORAMAC_FHDR_F_OPTS_MAX_FIELD_SIZE  15

/*! Port field size */
#define LORAMAC_F_PORT_FIELD_SIZE           1

/*! Port field size */
#define LORAMAC_MAC_PAYLOAD_FIELD_MAX_SIZE  242

/*! MIC field size */
#define LORAMAC_MIC_FIELD_SIZE              4

/*!
 * LoRaMAC header field definition (MHDR field)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2
 */
typedef union uLoRaMacHeader {
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sMacHeaderBits {
        /*!
         * Major version
         */
        uint8_t Major           : 2;
        /*!
         * RFU
         */
        uint8_t RFU             : 3;
        /*!
         * Message type
         */
        uint8_t MType           : 3;
    } Bits;
} LoRaMacHeader_t;

/*!
 * LoRaMAC frame types
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2.1, table 1
 */
typedef enum eLoRaMacFrameType {
    /*!
     * LoRaMAC join request frame
     */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /*!
     * LoRaMAC join accept frame
     */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /*!
     * LoRaMAC unconfirmed up-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /*!
     * LoRaMAC unconfirmed down-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /*!
     * LoRaMAC confirmed up-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /*!
     * LoRaMAC confirmed down-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /*!
     * LoRaMAC Rejoin Request
     */
    FRAME_TYPE_REJOIN                = 0x06,
    /*!
     * LoRaMAC proprietary frame
     */
    FRAME_TYPE_PROPRIETARY           = 0x07,
} LoRaMacFrameType_t;

/*!
 * LoRaMAC frame control field definition (FCtrl)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.3.1
 */
typedef union uLoRaMacFrameCtrl {
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sCtrlBits {
        /*!
         * Frame options length
         */
        uint8_t FOptsLen        : 4;
        /*!
         * Frame pending bit
         */
        uint8_t FPending        : 1;
        /*!
         * Message acknowledge bit
         */
        uint8_t Ack             : 1;
        /*!
         * ADR acknowledgment request bit
         */
        uint8_t AdrAckReq       : 1;
        /*!
         * ADR control in frame header
         */
        uint8_t Adr             : 1;
    } Bits;
} LoRaMacFrameCtrl_t;

/*!
 * LoRaMac Frame header (FHDR)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.3.1
 */
typedef struct sLoRaMacFrameHeader {
    /*!
     * Device address
     */
    uint32_t DevAddr;
    /*!
     * Frame control field
     */
    LoRaMacFrameCtrl_t FCtrl;
    /*!
     * Frame counter
     */
    uint16_t FCnt;
    /*!
     * FOpts field may transport  MAC commands (opt. 0-15 Bytes)
     */
    uint8_t FOpts[LORAMAC_FHDR_F_OPTS_MAX_FIELD_SIZE];
} LoRaMacFrameHeader_t;

/*!
 * LoRaMac type for Data MAC messages
 * (Unconfirmed Data Up, Confirmed Data Up, Unconfirmed Data Down, Confirmed Data Down)
 */
typedef struct sLoRaMacMessageData {
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint16_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     * Frame header (FHDR)
     */
    LoRaMacFrameHeader_t FHDR;
    /*!
     * Port field (opt.)
     */
    uint8_t FPort;
    /*!
     * Frame payload may contain MAC commands or data (opt.)
     */
    uint8_t* FRMPayload;
    /*!
     * Size of frame payload (not included in LoRaMac messages) 
     */
    uint8_t FRMPayloadSize;
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
} LoRaMacMessageData_t;

/*!
 * LoRaWAN Frame type enumeration to differ between the possible data up/down frame configurations.
 *
 * Note: The naming is implementation specific since there is no definition
 *       in the LoRaWAN specification included.
 */
typedef enum eFType {
    /*!
     * Frame type A
     *
     * FOptsLen > 0, Fopt present, FPort > 0, FRMPayload present
     */
    FRAME_TYPE_A,
    /*!
     * Frame type B
     *
     * FOptsLen > 0, Fopt present, FPort not present, FRMPayload not present
     */
    FRAME_TYPE_B,
    /*!
     * Frame type C
     *
     * FOptsLen = 0, Fopt not present, FPort = 0 , FRMPayload containing MAC commands
     */
    FRAME_TYPE_C,
    /*!
     * Frame type D
     *
     * FOptsLen = 0, Fopt not present, FPort > 0 , FRMPayload present
     */
    FRAME_TYPE_D,
} FType_t;

int8_t sx1303_gateway_start(bool pubic, uint8_t region);
int8_t sx1303_gateway_receive(void);

}

#endif