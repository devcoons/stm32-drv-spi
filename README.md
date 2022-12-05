# SPI Driver

SPI Driver reusable in different STM32 platforms utilizing DMA.

## Supported Hardware

- STM32L552 [PSU]

## How To Use

- Include the header file `drv_spi.h`
- Enable the SPI in the .ioc file
	- Configure the master normally (Enable DMA for RX/TX in normal mode)
	- Configure the slave as a circular buffer (Enable DMA for RX/TX in circular mode). This will allow automatic Transmit/Receive
- Create a `spi_t` instance. The following members of the structure must be defined:
	- `instance.handler`: STM32 HAL SPI handler(ex. hspi2)
	- `instance.mx_init`: STM32 HAL SPI MX_Init_SPI function (ex. MX_Init_SPI2())
	- `instance.type` : SPI COM Type `SPI_MASTER` or `SPI_SLAVE`
	- `instance.dt_rx`: Address of 8-bit array for reception buffer (+4 bytes which are used by the SPI Driver) 
	- `instance.dt_tx`: Address of 8-bit array for transmission buffer (+4 bytes which are used by the SPI Driver)
	- `instance.dt_sz` : `dt_rx,dt_tx` size (+4 bytes which are used by the SPI Driver)
	- `instance.GPIOx` : if `SPI_MASTER` : NSS Pin Port (hardware dependant) 
	- `instance.GPIO_Pin`:   if `SPI_MASTER` : NSS Pin Number (hardware dependant)
	
	
### Functions Guide
- `spi_initialize`: inizialized the peripheral (SPI must be enable before calling)
- `spi_deinitialize`: de-inizializes the peipheral
- `spi_start` : starts the peripheral 
- `spi_stop` : stops the peripheral
- `spi_send_msg` : sends the message. Using this with a SPI_SLAVE will raise an error.
- `spi_set`: sets sets the message to be sent (beware of the 2 leading and 2 ending SPI Driver bytes)
- `spi_get` : gets a received message (beware of the 2 leading and 2 ending SPI Driver bytes)


## Example
Let's consider an STM32 NUCLEO-L552ZE-Q with SPI2 enabled on full-duplex master mode.
The message received in saved in "received" and the one to be transmitted in "transmitted". The maximum size is 100.
The Chip Select line is on pin PB6. The spi_t structure would look like this:

-  Include the header file `drv_spi.h`
-  Define an `spi_t` handler and initialize/assign properly the structure.
```C
uint8_t tx_master_buf[1024+4] = {0};
uint8_t rx_master_buf[1024+4] = {0};

uint8_t tx_slave_buf[1024+4] = {0};
uint8_t rx_slave_buf[1024+4] = {0};

spi_t spi_master =
{
	.handler = &hspi1,
	.mx_init = MX_SPI1_Init,
	.type = SPI_MASTER,
	.dt_tx = tx_master_buf,
	.dt_rx = rx_master_buf,	
	.dt_sz = 1024+4,
	.nss_gpio_pin = GPIO_PIN_x,
	.nss_gpio_port = GPIOx,
};

spi_t spi_slave =
{
	.handler = &hspi2,
	.mx_init = MX_SPI2_Init,
	.type = SPI_SLAVE,
	.dt_tx = tx_slave_buf,
	.dt_rx = rx_slave_buf,	
	.dt_sz = 1024+4,
};
```

- Initialize and start the SPI:

```C

spi_initialize(&spi_master);
spi_send_msg(&spi_master);

// [...]
// [...]

spi_initialize(&spi_slave);
spi_start(&spi_slave)
```

- To set a message:

```C
uint8_t tx_message[] = {1,2,3,4,5,6,7,8};
spi_set(&spi_master, tx_message, sizeof(tx_message)/sizeof(uint8_t), 2);

// [...]
// [...]

uint8_t tx_message[] = {1,2,3,4,5,6,7,8};
spi_set(&spi_slave, tx_message, sizeof(tx_message)/sizeof(uint8_t), 2);
```

- To send a message (Only `SPI_MASTER`, in case of `SPI_SLAVE` the driver is automatically transmitting :

```C
spi_send_msg(&spi_master);
```


- To get a received message (could als include the SPI Driver bytes (2 leading and 2 ending):

```C
uint8_t rx_message[100];
spi_get(&spi_master,0,rx_message,16);
```

## Development & Contribution

Feel free to suggest anything using the Github ISSUES.

## License

This project is released under the MIT License
