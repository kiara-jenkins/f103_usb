Source Files for a nearly-generic USB device (2 bulk endpoints) on STM32F103
JLN 2026

	USB_CORE : files from the ST middleware, unmodified

	USB_CONF : user or class-specific files, derived from ST demos
		usbd_cla.* : class specific files, as much generic as possible (derived from CDC demo)
			configuration descriptor is there, class-specific but cannot be any generic
		usbd_conf.* : customizable function, according to ST, unmodified for the moment
		usbd_desc.* : application-specific descriptors, except configuration descriptor

	Drivers : USB device drivers from ST, HAL_PCD and LL_USB unmodified
		Note : these drivers do not really belong to the HAL and LL categories, they do not
		match the usual conventions for HAL and LL

include search paths (Atollic .cproject syntax):
	<listOptionValue builtIn="false" value="../Drivers/STM32F1xx_HAL_Driver/Inc"/>
	<listOptionValue builtIn="false" value="../USB_CONF"/>
	<listOptionValue builtIn="false" value="../USB_CORE"/>
	<listOptionValue builtIn="false" value="../Appli"/>
	<listOptionValue builtIn="false" value="C:/STM32/CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include"/>
	<listOptionValue builtIn="false" value="C:/STM32/CubeF1/Drivers/CMSIS/Include"/>
	<listOptionValue builtIn="false" value="C:/STM32/CubeF1/Drivers/STM32F1xx_HAL_Driver/Inc"/>

external linked source files :
	none