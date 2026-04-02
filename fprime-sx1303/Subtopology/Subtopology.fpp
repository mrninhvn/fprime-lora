module SX1303 {

    # SX1303 Radio Integration
    instance sx1303Manager: SX1303.SX1303Manager base id SX1303.SubtopologyConfig.BASE_ID + 0x1000 \
        queue size Components.QUEUE_SIZE \
        stack size Components.STACK_SIZE \
        priority 140

    # instance sx1303Manager: SX1303.SX1303Manager base id SX1303.SubtopologyConfig.BASE_ID + 0x1000

    topology Subtopology {
        instance sx1303Manager
        instance sx1303Driver
        
        connections SX1303 {
            sx1303Manager.spiReadWrite -> sx1303Driver.SpiReadWrite
        }
    }
}