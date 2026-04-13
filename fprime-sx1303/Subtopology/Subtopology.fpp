module SX1303 {

    # SX1303 Radio Integration
    instance sx1303Manager: SX1303.SX1303Manager base id SX1303.SubtopologyConfig.BASE_ID + 0x1000 \
        queue size Components.QUEUE_SIZE \
        stack size Components.STACK_SIZE \
        priority 140

    topology Subtopology {
        instance sx1303Manager
        instance sx1303SpiDriver
        instance sx1303PowerDriver
        instance sx1303ResetDriver
        instance sx1303DataProcessor
        
        connections SX1303 {
            sx1303Manager.spiReadWrite   -> sx1303SpiDriver.SpiReadWrite
            sx1303Manager.powerGpioWrite -> sx1303PowerDriver.gpioWrite
            sx1303Manager.resetGpioWrite -> sx1303ResetDriver.gpioWrite

            # Connect the SX1303 manager to the LoRa MAC processor
            sx1303Manager.loraOut -> sx1303DataProcessor.bufferLikeIn
        }
    }
}