# AT Commands

## Query all modules

```text
AT+ESP?
```

## Query one module

```text
AT+ESP=BUTTON,STATUS
AT+ESP=TIME,STATUS
AT+ESP=ETHERNET,STATUS
AT+ESP=WIFI_STA,STATUS
```

## Enable module

```text
AT+ESP=BUTTON,ENABLE,TRUE,TRUE
AT+ESP=ST7789V2,ENABLE,FALSE,TRUE
AT+ESP=KEYPAD,ENABLE,TRUE,FALSE
AT+ESP=ETHERNET,ENABLE,FALSE,TRUE
AT+ESP=TIME,ENABLE,FALSE,TRUE
AT+ESP=STEPPER,ENABLE,FALSE,TRUE
AT+ESP=WIFI_STA,ENABLE,FALSE,TRUE
```

Syntax:

```text
AT+ESP=<MODULE>,ENABLE[,LOG,AT]
```

Special shorthand for I2C:

```text
AT+ESP=I2C_MASTER,<SDA>,<SCL>,<HZ>
```

Behavior note for PCA9685:

- `AT+ESP=PCA9685,ENABLE,<LOG>,<AT>` now also applies the default PCA9685 configuration.
- After enable, the chip is ready without a separate `AT+PCA=CFG` in the common case.

Defaults when omitted:

- `LOG=FALSE`
- `AT=FALSE`

## Disable module

```text
AT+ESP=BUTTON,DISABLE
AT+ESP=ETHERNET,DISABLE
AT+ESP=TIME,DISABLE
AT+ESP=WIFI_STA,DISABLE
AT+ESP=STORAGE,DISABLE
```

## Supported module names

- `BUTTON`
- `CAN`
- `EASYLED`
- `ETHERNET`
- `GPIO`
- `I2C_MASTER`
- `KEYPAD`
- `OTA`
- `PCA9685`
- `RS485`
- `SCHEDULER`
- `SPI_MASTER`
- `STEPPER`
- `ST7789V2`
- `STORAGE`
- `TIME`
- `WIFI_STA`

