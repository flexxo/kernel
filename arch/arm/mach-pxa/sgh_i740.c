/* Support for PXA310-based devices, 
* RoverPC Pro G8, SimCom PW10, Dec S90, RoverPC Evo X8, Qigi U86 etc...
* Now supported:
* - Screen
* - Keyboard
* - Sound (It works, but I can't test it.)
* - Touchscreen (Works, but didn't calibrated)
* - Battery&Charger (Charger works, but we haven't info about battery controller)
* Based on zylonite.c (C) Marvell
* Based on sgh_i900_i780.c (C) Guys from andromnia project
* Based on t5388p.c (C) Anton Radzhabli 
* This sources is deprecated.
* Now I use new type of kernel, that will be support bigger number of devices, rly.
* Editor me.flexxo <me.flexxo@gmail.com>
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/power_supply.h>
#include <linux/pda_power.h>
#include <../drivers/staging/android/timed_gpio.h>

#include <plat/i2c.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/hardware.h>
#include <mach/pxafb.h>
#include <mach/audio.h>
#include <mach/mmc.h>
#include <mach/udc.h>
#include <mach/ohci.h>
#include <mach/pxa27x-udc.h>
#include <mach/pxa27x_keypad.h>
#include <mach/pxa2xx_spi.h>
#include <mach/pxa3xx-regs.h>
#include <mach/mfp-pxa300.h>
#include <mach/irqs.h>

#include "devices.h"
#include "generic.h" 

/********************************************************
*         Multi-Function Pins defining 			*
********************************************************/
static mfp_cfg_t sgh_i740_mfp_cfg[] __initdata = { 
/* WILL BE HERE SOON*/
};
/********************************************************
*         End of  Multi-Function Pins defining 		*
********************************************************/

/********************************************************
*     Liquid Crystal Display properties defining 	*
********************************************************/
#if defined(CONFIG_FB_PXA) || defined(CONFIG_FB_PXA_MODULE) 
static struct platform_pwm_backlight_data sgh_i740_backlight_data = {
	.pwm_id		= 2,
	.max_brightness	= 100,
	.dft_brightness	= 80,
	.pwm_period_ns	= 10000,
};

static struct platform_device sgh_i740_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent = &pxa27x_device_pwm1.dev,
		.platform_data	= &sgh_i740_backlight_data,
	},
};

static struct pxafb_mode_info sgh_i740_mode = {
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.hsync_len	= 2,
	.left_margin	= 11,
	.right_margin	= 11,
	.vsync_len	= 1,
	.upper_margin	= 10,
	.lower_margin	= 10,
	.sync		= 0,
}; 
/* Rover G8's LCCR3 modes :
1430000f  //PCD = 15
14300017 //PCD = 23
1430001f  //PCD = 31
*/

static struct pxafb_mach_info sgh_i740_lcd_info = {
	.modes = &sgh_i740_mode,
	.num_modes = 1,
	.lcd_conn = 0,
	.lccr0 = 0x03b008f9,
	.lccr3 = 0x1430000f, // I'm set this option, cuz this is no another way to start the LCD in the correct way.
	.lccr4 = 0x84007e00,
}; 

static void __init sgh_i740_init_lcd(void) 
{
	platform_device_register(&sgh_i740_backlight_device);
	set_pxa_fb_info(&sgh_i740_lcd_info);
}
#else
static inline void sgh_i740_init_lcd(void) {} 
#endif
/********************************************************
*   Liquid Crystal Display properties defining end	*
********************************************************/


/********************************************************
* 		 SD Card block				*
********************************************************/
#if defined(CONFIG_MMC)
static struct pxamci_platform_data sgh_i740_mci_platform_data = {
	.detect_delay		= 20,
	.ocr_mask		= MMC_VDD_32_33 | MMC_VDD_33_34,
	.gpio_card_ro		= -1,
	.gpio_power		= -1,
};

static void __init sgh_i740_init_mmc(void)
{
	pxa_set_mci_info(&sgh_i740_mci_platform_data);
}
#else
static inline void sgh_i740_init_mmc(void) {}
#endif
/********************************************************
* 		SD Card block end			*
********************************************************/

/********************************************************
* 		Keyboard block				*
********************************************************/
#if defined(CONFIG_KEYBOARD_PXA27x)
static unsigned int sgh_i740_matrix_key_map[] = {
	/*Rover G8 Mode */
	/* KEY(row, col, key_code) */
	KEY(5, 1, KEY_CAMERAFOCUS),	//Camera half
	KEY(5, 2, KEY_VOLUMEUP),	//Volume up
	KEY(5, 3, KEY_VOLUMEDOWN),	//Volume down
	KEY(5, 4, KEY_CAMERA),		//Camera full
	KEY(0, 3, KEY_BACK),		//Green button
	KEY(4, 0, KEY_MENU),		//Windows key
	KEY(1, 0, KEY_ENTER),		//OK button
	KEY(1, 3, KEY_HOME),		//Red button
};

static struct pxa27x_keypad_platform_data sgh_i740_keypad_info = {
	.enable_rotary0 	= 0,
	.debounce_interval	= 30,
	.matrix_key_rows	= 6,
	.matrix_key_cols	= 5,
	.matrix_key_map = sgh_i740_matrix_key_map,
	.matrix_key_map_size = ARRAY_SIZE(sgh_i740_matrix_key_map),
};

static void __init sgh_i740_init_keypad(void)
{
	pxa_set_keypad_info(&sgh_i740_keypad_info);
}
#else
static inline void sgh_i740_init_keypad(void) {}
#endif
/********************************************************
* 		Keyboard block end			*
********************************************************/

/********************************************************
* 		Platform devices defining		*
********************************************************/
/* WILL BE HERE SOON TOO */
/********************************************************
*	     Platform devices defining end	       	*
********************************************************/

/********************************************************
* 	       Devices initialization		 	*
********************************************************/
static void __init sgh_i740_init(void)
{
	pxa3xx_mfp_config(ARRAY_AND_SIZE(sgh_i740_mfp_cfg));;
	sgh_i740_init_lcd();
	sgh_i740_init_mmc();
	sgh_i740_init_keypad();
//	pxa_set_ac97_info(NULL); Рано еще
	platform_add_devices(devices, ARRAY_SIZE(devices));
}
/********************************************************
* 	      Devices initialization end	  	*
********************************************************/

// Machine INIT...
MACHINE_START(SGH_I740, "Samsung SGH-i740")
        .phys_io        = 0x40000000,
        .boot_params    = 0xa0000100,
        .io_pg_offst    = (io_p2v(0x40000000) >> 18) & 0xfffc,
        .map_io         = pxa_map_io,
        .init_irq       = pxa3xx_init_irq,
        .timer          = &pxa_timer,
        .init_machine   = sgh_i740_init,
MACHINE_END 
