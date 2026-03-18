# AT Commands

## Query all modules

```text
AT+ESP?
```

## Query one module

```text
AT+ESP=BUTTON,STATUS
AT+ESP=NETWORK,STATUS
```

## Enable module

```text
AT+ESP=BUTTON,ENABLE,TRUE,TRUE
AT+ESP=ST7789V2,ENABLE,FALSE,TRUE
AT+ESP=KEYPAD,ENABLE,TRUE,FALSE
```

Syntax:

```text
AT+ESP=<MODULE>,ENABLE[,LOG,AT]
```

Defaults when omitted:

- `LOG=FALSE`
- `AT=FALSE`

## Disable module

```text
AT+ESP=BUTTON,DISABLE
AT+ESP=NETWORK,DISABLE
AT+ESP=STORAGE,DISABLE
```

## Supported module names

- `BUTTON`
- `CAN`
- `EASYLED`
- `GPIO`
- `I2C_MASTER`
- `KEYPAD`
- `NETWORK`
- `OTA`
- `RS485`
- `SCHEDULER`
- `SPI_MASTER`
- `ST7789V2`
- `STORAGE`
