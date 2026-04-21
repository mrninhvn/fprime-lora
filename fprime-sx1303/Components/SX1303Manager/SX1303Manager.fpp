module SX1303 {
    active component SX1303Manager {

        enum gwState { POWER_OFF, POWER_ON, RESET, CHIP_ID_CHECK, CONFIGURE, RUNNING };
        enum gwChPlan: U8 {
            EU868   = 1,
            US915   = 2,
            CN779   = 3,
            EU433   = 4,
            AU915   = 5,
            CN470   = 6,
            AS923   = 7,
            AS923_2 = 8,
            AS923_3 = 9,
            KR920   = 10,
            IN865   = 11,
            RU864   = 12,
            AS923_4 = 13
        }

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

        @ Command to turn on or off the gateway
        async command GW_ON_OFF(
            onOff: Fw.On @< Indicates whether the gateway should be on or off
        )

        @ Reset gateway
        async command GW_RESET

        @ Lora test transmission command
        async command GW_TEST_TX

        @ Gateway start command
        async command GW_START(
            publicNet: bool, @< Is Public network
            region: gwChPlan @< Channel Plan ID
        )

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # @ Example async command
        # async command COMMAND_NAME(param_name: U32)

        # @ Example telemetry counter
        # telemetry ExampleCounter: U64
        # @ Telemetry channel to report blinking state.
        # telemetry BlinkingState: Fw.On

        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        @ Debug log
        event Debug(msg: string size 200) severity activity low format "Debug: {}"

        @ Gateway state event, produced whenever the gateway state changes
        event GwState(new_state: gwState) severity activity high format "SX1303 State: {}"

        @ Example port: receiving calls from the rate group
        sync input port run: Svc.Sched

        @ Port for reset control
        output port resetGpioWrite: Drv.GpioWrite

        @ Port for Power-en control
        output port powerGpioWrite: Drv.GpioWrite

        @ Port for SPI bus communication
        output port spiReadWrite: Drv.SpiReadWrite

        # @ Port for sending packet data to Lora MAC processor
        # output port loraOut: Fw.BufferSend

        @ Telemetry channel to report gateway state.
        telemetry GatewayState: gwState

        @ Telemetry channel for received LoRa packet metadata
        telemetry GatewayPacket: SX1303Data

        #@ Example parameter
        #param PARAMETER_NAME: U32

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}