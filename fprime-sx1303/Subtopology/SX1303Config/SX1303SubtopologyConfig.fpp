module SX1303 {
    module SubtopologyConfig {
        constant BASE_ID = 0xD0000000
    }

    module Components {
        constant QUEUE_SIZE = 10
        constant STACK_SIZE = 64 * 1024
    }

    instance sx1303PowerDriver: Drv.LinuxGpioDriver base id SX1303.SubtopologyConfig.BASE_ID + 0x00002000
    instance sx1303ResetDriver: Drv.LinuxGpioDriver base id SX1303.SubtopologyConfig.BASE_ID + 0x00003000

    instance sx1303SpiDriver: Drv.LinuxSpiDriver base id SX1303.SubtopologyConfig.BASE_ID + 0x00004000 {
        phase Fpp.ToCpp.Phases.configComponents """
        if (not SX1303::sx1303SpiDriver.open(state.sx1303.device.device, state.sx1303.device.select, Drv::SPI_FREQUENCY_5MHZ)) {
            Fw::Logger::log("[ERROR] SX1303 SPI open failed\\n");
        }
        else {
            Fw::Logger::log("[INFO] SX1303 SPI open successful\\n");
        }
        """
    }

    # @ Lora MAC processor to received data from SX1303 and process it for LoRaWAN
    # instance sx1303DataProcessor: LORAMAC.LoRaMacProcessor base id SX1303.SubtopologyConfig.BASE_ID + 0x5000
} 