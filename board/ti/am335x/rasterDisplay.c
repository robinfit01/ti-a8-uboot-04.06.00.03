/*
 * rasterDisplay.c
 *
 * Sample application for raster
 *
*/

/* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include "soc_AM335x.h"
#include "interrupt.h"
#include "evmAM335x.h"
#include "raster.h"
#include "image.h"
#include "ecap.h"
#include "hsi2c.h"

#include <common.h>
#include <asm/cache.h>
#include <asm/omap_common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <asm/arch/clock.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>
#include <spi_flash.h>
#include "common_def.h"
#include "pmic.h"
#include "tps65217.h"
#include <i2c.h>
#include <serial.h>
#include <malloc.h>
#include "hw_types.h"



#define WIDTH 800
#define HEIGHT  600

#define HFP   16
#define HBP   100
#define HSW   100

#define VFP   40
#define VBP   3
#define VSW   20

#define FRESH_HZ  60


#if 0
unsigned int image2[1024*600+8] __attribute__((aligned(4)))= {
0x4000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u,
};
#endif
//extern unsigned char logo_linux_clut224_data[615096];
//extern unsigned char logo_linux_clut224_clut[660];
extern unsigned char logo_linux_clut224_data[];
extern unsigned char logo_linux_clut224_clut[];

#define M_IMAGE_SIZE (WIDTH*HEIGHT+8)
unsigned int *image1;

/******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
static void LCDIsr(void);
static void SetUpLCD(void);
static void LCDAINTCConfigure(void);
/******************************************************************************
**              FUNCTION DEFINITIONS
******************************************************************************/
static unsigned long cnt1=0;
static unsigned long cnt2=0;


unsigned int g_u32_lcd_reg[] = {
	LCDC_PID  		,
	LCDC_LCD_CTRL   	,	
	LCDC_LCD_STAT   	,
	LCDC_LIDD_CTRL  	,
	LCDC_LIDD_CS0_CONF  	,
	LCDC_LIDD_CS0_ADDR  	,
	LCDC_LIDD_CS0_DATA  	,
	LCDC_LIDD_CS1_CONF   	,
	LCDC_LIDD_CS1_ADDR  	,
	LCDC_LIDD_CS2_DATA   	,
	LCDC_RASTER_CTRL	,   
	LCDC_RASTER_TIMING_0    ,
	LCDC_RASTER_TIMING_1    ,
	LCDC_RASTER_TIMING_2    ,
	LCDC_RASTER_SUBPANEL   	,
	LCDC_RASTER_SUBPANEL2   ,
	LCDC_LCDDMA_CTRL   	,
	LCDC_LCDDMA_FB0_BASE  	,
	LCDC_LCDDMA_FB0_CEILING , 
	LCDC_LCDDMA_FB1_BASE  	,
	LCDC_LCDDMA_FB1_CEILING ,
	LCDC_SYSCONFIG   	,
	LCDC_IRQSTATUS_RAW   	,
	LCDC_IRQSTATUS  	,
	LCDC_IRQENABLE_SET  	,
	LCDC_IRQENABLE_CLEAR  	,
	LCDC_IRQEOI_VECTOR   	,
	LCDC_CLKC_ENABLE  	,
	LCDC_CLKC_RESET  	
};

char * g_ps8_lcd_reg[] = {
	"LCDC_PID"  		,
	"LCDC_LCD_CTRL"   	,	
	"LCDC_LCD_STAT"   	,
	"LCDC_LIDD_CTRL"  	,
	"LCDC_LIDD_CS0_CONF"  	,
	"LCDC_LIDD_CS0_ADDR"  	,
	"LCDC_LIDD_CS0_DATA"  	,
	"LCDC_LIDD_CS1_CONF"   	,
	"LCDC_LIDD_CS1_ADDR"  	,
	"LCDC_LIDD_CS2_DATA"   	,
	"LCDC_RASTER_CTRL"	,   
	"LCDC_RASTER_TIMING_0"   ,
	"LCDC_RASTER_TIMING_1"   ,
	"LCDC_RASTER_TIMING_2"   ,
	"LCDC_RASTER_SUBPANEL"   ,
	"LCDC_RASTER_SUBPANEL2"  ,
	"LCDC_LCDDMA_CTRL"   	,
	"LCDC_LCDDMA_FB0_BASE"  ,
	"LCDC_LCDDMA_FB0_CEILING", 
	"LCDC_LCDDMA_FB1_BASE"  ,
	"LCDC_LCDDMA_FB1_CEILING",
	"LCDC_SYSCONFIG"   	,
	"LCDC_IRQSTATUS_RAW"   	,
	"LCDC_IRQSTATUS" 	,
	"LCDC_IRQENABLE_SET"  	,
	"LCDC_IRQENABLE_CLEAR"  ,
	"LCDC_IRQEOI_VECTOR"   	,
	"LCDC_CLKC_ENABLE"  	,
	"LCDC_CLKC_RESET"  	
};


void fn_show_lcd_reg_data(void)
{
	int i;
	int j;
	j = sizeof(g_u32_lcd_reg)/sizeof(unsigned int);
	for(i = 0;i < j;i++)
	{
		printf("%s=%d,data=0x%08x\n",g_ps8_lcd_reg[i],g_u32_lcd_reg[i],RasterGetRegData(g_u32_lcd_reg[i]));
	}
}

	
int fn_lcd_init(void)
{
    //IntMasterIRQEnable();

    //IntAINTCInit();

    //LCDAINTCConfigure();

    //LCDBackLightEnable();

    //UPDNPinControl();

#if (LSD_DEBUG > 0)
	printf("lierda debug:enter File=board/ti/am335x/rasterDisplay.c,Function=fn_lcd_init line1\n");	
#endif
	
    SetUpLCD();

    image1 = (unsigned int*)(0x80A00000);

  
    /* Configuring the base ceiling */
    RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                      (unsigned int)image1,
                      //(unsigned int)image1 + sizeof(image1) - 2,
			(unsigned int)image1 + M_IMAGE_SIZE*4 - 2,
                      0);

    RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                      (unsigned int)image1,
	              //(unsigned int)image1 + sizeof(image1) - 2,
			(unsigned int)image1 + M_IMAGE_SIZE*4 - 2,
                      1);

    /* Enable End of frame0/frame1 interrupt */
    RasterIntEnable(SOC_LCDC_0_REGS, RASTER_END_OF_FRAME0_INT |
                                     RASTER_END_OF_FRAME1_INT);

    /* Enable raster */
    RasterEnable(SOC_LCDC_0_REGS);

#if (LSD_DEBUG > 0)
	printf("lierda debug:enter File=board/ti/am335x/rasterDisplay.c,Function=fn_lcd_init line2\n");	
	//printf("lierda debug:enter File=board/ti/am335x/rasterDisplay.c,Function=fn_lcd_init image2 size=%d\n",sizeof(image2));
	//printf("lierda debug:enter File=board/ti/am335x/rasterDisplay.c,Function=fn_lcd_init image2 addr=0x%08x\n",(unsigned long)image2);
#endif

    //image1 = (unsigned int*)malloc(M_IMAGE_SIZE*4);

    unsigned long i;
    unsigned int *p_image;
    p_image = image1;

    unsigned char r,r_tmp;
    unsigned char g,g_tmp;
    unsigned char b,b_tmp;
    unsigned char tmp;
    unsigned long u32_tmp;
    unsigned long u32_array;
    *(p_image++) = 0x4000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    *(p_image++) = 0x0000;
    for(i = 0; i < (HEIGHT*WIDTH); i++)
    {
        tmp =  logo_linux_clut224_data[i];
#if 0
	r_tmp = (tmp >> 5) & 0x07;
	g_tmp = (tmp >> 2) & 0x07;
	b_tmp = (tmp >> 0) & 0x03;
	r = (r_tmp << 5) | (r_tmp << 2) | (r_tmp >> 1);
	g = (g_tmp << 5) | (g_tmp << 2) | (g_tmp >> 1);
	b = (b_tmp << 6) | (b_tmp << 4) | (b_tmp << 2) | (b_tmp << 0);
#endif
#if 0
	r_tmp = (tmp >> 6) & 0x03;
	g_tmp = (tmp >> 4) & 0x03;
	b_tmp = (tmp >> 0) & 0x0F;
	r = (r_tmp << 6) | (r_tmp << 4) | (r_tmp << 2) | (r_tmp << 0);
	g = (g_tmp << 6) | (g_tmp << 4) | (g_tmp << 2) | (g_tmp << 0);
	b = (b_tmp << 4) | (b_tmp << 0);
#endif
// use this normal
#if 1
	tmp = tmp - 32;
	u32_array = tmp;
	if(u32_array > 220)
		u32_array = 220;
	u32_array = u32_array*3;
	r = logo_linux_clut224_clut[u32_array+0];
	g = logo_linux_clut224_clut[u32_array+1];
	b = logo_linux_clut224_clut[u32_array+2];
	u32_tmp = ((b << 16) | (g << 8) | (r))&0x00FFFFFF;
	*(p_image) = u32_tmp;
        p_image++;
#endif	

	
#if 0
// 通过这种方式确认RGB排布顺序
// 此时显示的结果是 0x00 Blue Green Red
    for(i = 8; i < (1024*600)/3+8; i++)
    {
         *(p_image) = 0x000000FF;
         p_image++;
    }
    for(; i < (2*1024*600)/3+8; i++)
    {
         *(p_image) = 0x0000FF00;
         p_image++;
    }
    for(; i < (1024*600)+8; i++)
    {
         *(p_image) = 0x00FF0000;
         p_image++;
    }
#endif

#if 0
    // 此时显示的结果是 0x00 Blue Green Red
         *(p_image) = 0x000000FF;
         p_image++;
    
#endif

	
	
    }
    
	
    while(1)
    {
	LCDIsr();
	if((cnt1 > 10) && (cnt2 > 10))
	{
		fn_show_lcd_reg_data();
		break;
	}
		
    }
}




/*
** Configures raster to display image 
*/
static void SetUpLCD(void)
{
    /* Enable clock for LCD Module */ 
    LCDModuleClkConfig();

    LCDPinMuxSetup();

    /* 
    **Clock for DMA,LIDD and for Core(which encompasses
    ** Raster Active Matrix and Passive Matrix logic) 
    ** enabled.
    */
    RasterClocksEnable(SOC_LCDC_0_REGS);

    /* Disable raster */
    RasterDisable(SOC_LCDC_0_REGS);
    
    /* Configure the pclk */
    //RasterClkConfig(SOC_LCDC_0_REGS, 23040000, 192000000);
    //RasterClkConfig(SOC_LCDC_0_REGS, (WIDTH+HFP+HBP+HSW)*(HEIGHT+VFP+VBP+VSW)*FRESH_HZ, 192000000);
	RasterClkConfig(SOC_LCDC_0_REGS, 40000000, 192000000);

    /* Configuring DMA of LCD controller */ 
    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

    /* Configuring modes(ex:tft or stn,color or monochrome etc) for raster controller */
    RasterModeConfig(SOC_LCDC_0_REGS, RASTER_DISPLAY_MODE_TFT_UNPACKED,
                     RASTER_PALETTE_DATA, RASTER_COLOR, RASTER_RIGHT_ALIGNED);


     /* Configuring the polarity of timing parameters of raster controller */
    RasterTiming2Configure(SOC_LCDC_0_REGS, RASTER_FRAME_CLOCK_LOW |
                                            RASTER_LINE_CLOCK_LOW  |
                                            RASTER_PIXEL_CLOCK_HIGH|
                                            RASTER_SYNC_EDGE_RISING|
                                            RASTER_SYNC_CTRL_ACTIVE|
                                            RASTER_AC_BIAS_HIGH     , 0, 255);


    /* Configuring horizontal timing parameter */
    RasterHparamConfig(SOC_LCDC_0_REGS, WIDTH, HSW, HFP, HBP);
	//RasterHparamConfig(SOC_LCDC_0_REGS, 1024, 48, 40, 40);

    /* Configuring vertical timing parameters */
    RasterVparamConfig(SOC_LCDC_0_REGS, HEIGHT, VSW, VSW, VBP);
	//RasterVparamConfig(SOC_LCDC_0_REGS, 600, 3, 13, 29);


    RasterFIFODMADelayConfig(SOC_LCDC_0_REGS, 128);


#if 0
	HWREG(SOC_LCDC_0_REGS + LCDC_RASTER_TIMING_0) = 0x2ec8a0f8;
	HWREG(SOC_LCDC_0_REGS + LCDC_RASTER_TIMING_1) = 0x0328531f;	
	HWREG(SOC_LCDC_0_REGS + LCDC_RASTER_TIMING_2) = 0x0230ff00;
	HWREG(SOC_LCDC_0_REGS + LCDC_RASTER_CTRL) = 0x001ff081;
	
	//HWREG(SOC_LCDC_0_REGS + LCDC_IRQENABLE_SET) = 0x00000360;
#endif

}


/*
** configures arm interrupt controller to generate raster interrupt 
*/
static void LCDAINTCConfigure(void)
{
    /* Register the ISR in the Interrupt Vector Table.*/
    IntRegister(SYS_INT_LCDCINT, LCDIsr);

    IntPrioritySet(SYS_INT_LCDCINT, 0, AINTC_HOSTINT_ROUTE_IRQ );

    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(SYS_INT_LCDCINT);
}

/*
** For each end of frame interrupt base and ceiling is reconfigured 
*/
static void LCDIsr(void)
{
    unsigned int  status;
    

    status = RasterIntStatus(SOC_LCDC_0_REGS,RASTER_END_OF_FRAME0_INT_STAT |
                                             RASTER_END_OF_FRAME1_INT_STAT );

    status = RasterClearGetIntStatus(SOC_LCDC_0_REGS, status);   

    if (status & RASTER_END_OF_FRAME0_INT_STAT)
    {
	cnt1++;	
	#if (LSD_DEBUG > 0)
	if((cnt1%10)==0)
	{
		printf("lierda debug: RASTER_END_OF_FRAME0_INT_STAT\n");
		//cnt1 = 0;
	}
	#endif        
	RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                          (unsigned int)image1,
                          //(unsigned int)image1 + sizeof(image1) - 2,
			  (unsigned int)image1 + M_IMAGE_SIZE*4 - 2,
                          0);
    }

    if(status & RASTER_END_OF_FRAME1_INT_STAT)
    {
	cnt2++;
	#if (LSD_DEBUG > 0)
	if((cnt2%10)==0)
	{
		printf("lierda debug: RASTER_END_OF_FRAME1_INT_STAT\n");
		//cnt2 = 0;
	}
	#endif           
	RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                          (unsigned int)image1,
                          //(unsigned int)image1 + sizeof(image1) - 2,
			  (unsigned int)image1 + M_IMAGE_SIZE*4 - 2,
                          1);
    }

    RasterEndOfInterrupt(SOC_LCDC_0_REGS, 0);
}

/***************************** End Of File ************************************/
