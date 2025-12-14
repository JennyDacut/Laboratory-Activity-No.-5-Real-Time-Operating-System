Def GPIO_InitStruct = {0};

    // Enable GPIO Clock
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Default pin configuration
    GPIO_InitStruct.Pin = AHT_SCL_PIN | AHT_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(AHT_SCL_PORT, &GPIO_InitStruct);

    SCL_H; SDA_H;
    HAL_Delay(100); // Power-on delay required by datasheet

    // Send Calibration Command
    I2C_Start();
    I2C_SendByte(AHT_ADDR << 1); // Address + Write bit
    if(!I2C_WaitAck()) {
        I2C_SendByte(0xBE); // Init Command
        I2C_WaitAck();
        I2C_SendByte(0x08); // Parameter 1
        I2C_WaitAck();
        I2C_SendByte(0x00); // Parameter 2
        I2C_WaitAck();
        I2C_Stop();
    }
    HAL_Delay(10);
}

// Read Temperature and Humidity
uint8_t AHT21_Read(float *Temperature, float *Humidity) {
    uint8_t data[6];

    // 1. Trigger Measurement Command (0xAC)
    I2C_Start();
    I2C_SendByte(AHT_ADDR << 1);
    if(I2C_WaitAck()) return 0; // Check for device presence
    I2C_SendByte(0xAC); // Trigger Measure
    if(I2C_WaitAck()) return 0;
    I2C_SendByte(0x33); // Param 1
    if(I2C_WaitAck()) return 0;
    I2C_SendByte(0x00); // Param 2
    if(I2C_WaitAck()) return 0;
    I2C_Stop();

    // 2. Wait for Measurement (Sensor needs ~80ms to process)
    HAL_Delay(80);

    // 3. Read Data (6 Bytes)
    I2C_Start();
    I2C_SendByte((AHT_ADDR << 1) | 1); // Address + Read bit
    if(I2C_WaitAck()) return 0;

    for(int i=0; i<6; i++) {
        data[i] = I2C_ReadByte(i < 5); // ACK first 5 bytes, NACK last one
    }
    I2C_Stop();

    // 4. Parse Data (Bit shifting)
    // The sensor sends 20-bit values split across bytes. We assume the data is valid.
    // Humidity: Combined from Byte 1, Byte 2, and top 4 bits of Byte 3
    uint32_t rawHum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((data[3] & 0xF0) >> 4);
    // Temperature: Combined from lower 4 bits of Byte 3, Byte 4, and Byte 5
    uint32_t rawTemp = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 😎 | data[5];

    // 5. Convert to Floating Point (Formulas from Datasheet)
    *Humidity = (float)rawHum * 100.0f / 1048576.0f;
    *Temperature = ((float)rawTemp * 200.0f / 1048576.0f) - 50.0f;

    return 1;
}
