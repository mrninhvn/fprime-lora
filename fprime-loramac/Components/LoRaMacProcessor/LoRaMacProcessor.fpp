module LORAMAC {
    @ Adapter for FrameAccumulator for use outside of communications
    passive component LoRaMacProcessor {

        @ Port to receive buffer like data
        sync input port bufferLikeIn: Fw.BufferSend

        @ Port for sending buffer like data
        output port bufferLikeOut: Fw.BufferSend

        @ Port to receive comm like data
        sync input port commLikeIn: Svc.ComDataWithContext

        @ Port to receive byte stream like data
        sync input port byteStreamLikeIn: Drv.ByteStreamData

        @ Port for sending comm like data
        output port commLikeOut: Svc.ComDataWithContext

        @ Port for requesting the current time
        time get port timeCaller

        @ Debug log
        event Debug(msg: string size 200) severity activity low format "LoRa MAC: {}"
        
        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut
    }
}