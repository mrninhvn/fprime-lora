module SX1303 {
    active component SX1303Manager {

        enum gwState { POWER_OFF, POWER_ON, RESET, STARTUP_DELAY, CHIP_ID_CHECK, CALIBRATION_READ, CONFIGURE, RUNNING };

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

        @ Command to turn on or off the gateway
        async command GW_ON_OFF(
            onOff: Fw.On @< Indicates whether the gateway should be on or off
        )

        @ Reset gateway
        async command GW_RESET

        @ Start gateway
        async command GW_TEST_TX

        @ Report the radio serial number
        async command ReportNodeIdentifier

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # @ Example async command
        # async command COMMAND_NAME(param_name: U32)

        # @ Example telemetry counter
        # telemetry ExampleCounter: U64
        # @ Telemetry channel to report blinking state.
        # telemetry BlinkingState: Fw.On

        # @ Telemetry channel to report LedTransitions.
        # telemetry LedTransitions: U64

        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        @ Produces a node-identifier
        event RadioNodeIdentifier(
              identifier: string size 20 @< Radio identifier
        ) \
        severity activity high \
        id 0 \
        format "Radio identification: {}"

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

        @ Telemetry channel to report gateway state.
        telemetry GatewayState: gwState

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