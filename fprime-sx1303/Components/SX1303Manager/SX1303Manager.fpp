module SX1303 {
    active component SX1303Manager {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

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

        @ Example port: receiving calls from the rate group
        sync input port run: Svc.Sched

        @ Port for SPI bus communication
        output port spiReadWrite: Drv.SpiReadWrite

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