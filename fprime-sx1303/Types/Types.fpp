module SX1303 {

    @ Struct representing SX1303 sensor data
    struct SX1303Data {
        @ Pressure in Pascals (Pa)
        pressure: F32
        
        @ Temperature in degrees Celsius (°C)
        temperature: F32
        
        @ Altitude in meters (m)
        altitude: F32
    }
} 