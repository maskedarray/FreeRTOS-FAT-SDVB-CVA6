
#include <FreeRTOS.h>
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_ramdisk.h"
#include "ff_sys.h"
#include <task.h>
#include <uart/uart.h>
#include "stdio.h"
#include <unistd.h>
#include "disparity_top.h"
#include "mser.h"
#include "pca.h"


#define mainRAM_DISK_SECTOR_SIZE     512UL                                                    /* Currently fixed! */
#define mainRAM_DISK_SECTORS         ( ( 5UL * 1024UL * 1024UL ) / mainRAM_DISK_SECTOR_SIZE ) /* 5M bytes. */
#define mainIO_MANAGER_CACHE_SIZE    ( 15UL * mainRAM_DISK_SECTOR_SIZE )
#define mainRAM_DISK_NAME            "/ram"

static uart_instance_t * const gp_my_uart = &g_uart_0;
static int g_stdio_uart_init_done = 0;



void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

/* Prepare hardware to run the demo. */
static void prvSetupHardware( void );

/* Send a message to the UART initialised in prvSetupHardware. */
void vSendString( const char * const pcString );
static void printTask( void *pvParameters );

uint8_t s_uart[12] = "init_uart\r\n";
extern void freertos_risc_v_trap_handler( void );

uint8_t print_buffer[256];
static void prvSetupHardware( void )
{

    UART_init(gp_my_uart,
              STDIO_BAUD_RATE,
              UART_DATA_8_BITS | UART_NO_PARITY | UART_ONE_STOP_BIT);
                      
    g_stdio_uart_init_done = 1;
    UART_polled_tx_string(gp_my_uart, s_uart);


	//PLIC_init();
	//UART_init( &g_uart, COREUARTAPB0_BASE_ADDR, BAUD_VALUE_115200, ( DATA_8_BITS | NO_PARITY ) );
}


void DIRCommand( const char *pcDirectoryToScan )
{
FF_FindData_t *pxFindStruct;
const char  *pcAttrib,
            *pcWritableFile = "writable file",
            *pcReadOnlyFile = "read only file",
            *pcDirectory = "directory";

	sprintf(print_buffer, "\r\n\n<===========================>\r\n");
	UART_polled_tx_string(gp_my_uart, print_buffer);

    /* FF_FindData_t can be large, so it is best to allocate the structure
    dynamically, rather than declare it as a stack variable. */
    pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );

    /* FF_FindData_t must be cleared to 0. */
    memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );

    /* The first parameter to ff_findfist() is the directory being searched.  Do
    not add wildcards to the end of the directory name. */
    if( ff_findfirst( pcDirectoryToScan, pxFindStruct ) == 0 )
    {
        do
        {
            /* Point pcAttrib to a string that describes the file. */
            if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
            {
                pcAttrib = pcDirectory;
            }
            else if( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
            {
                pcAttrib = pcReadOnlyFile;
            }
            else
            {
                pcAttrib = pcWritableFile;
            }

            /* Print the files name, size, and attribute string. */
            sprintf(print_buffer, "%s [%s] [size=%d] \r\n", pxFindStruct->pcFileName,
                                                  pcAttrib,
                                                  pxFindStruct->ulFileSize );
			UART_polled_tx_string(gp_my_uart, print_buffer);

        } while( ff_findnext( pxFindStruct ) == 0 );
    }

	sprintf(print_buffer, "<============================>\r\n\n");
	UART_polled_tx_string(gp_my_uart, print_buffer);

    /* Free the allocated FF_FindData_t structure. */
    vPortFree( pxFindStruct );
}

extern uint8_t my_fat_image_start;
extern uint8_t my_fat_image_end;
extern const int my_fat_image_size;

int main( void )
{
	prvSetupHardware();


	__asm__ volatile( "csrw mtvec, %0" :: "r"( freertos_risc_v_trap_handler ) );
	 

    // Use sprintf to format a string into the print_buffer
    // sprintf((char*)print_buffer, "Hello, ! The answer is %d.\r\n", 42);
	// UART_polled_tx_string(gp_my_uart, print_buffer);

	// int32_t cache_buf[1024*1024];
	// for (int i = 0; i < 1024*1024; i++){
	// 	cache_buf[i] = cache_buf[i] -2;
	// }
	static uint8_t ucRAMDisk[ mainRAM_DISK_SECTORS * mainRAM_DISK_SECTOR_SIZE ];
    FF_Disk_t * pxDisk;
	FF_FILE *pxSourceFile, *pxDestinationFile;

    /* Create the RAM disk. */
    pxDisk = FF_RAMDiskInit2( mainRAM_DISK_NAME, &my_fat_image_start, 102400, mainIO_MANAGER_CACHE_SIZE );
	FF_RAMDiskShowPartition(pxDisk);
	DIRCommand("/ram/");
	DIRCommand("/ram/disp");

	benchmark_disp();
	
	DIRCommand("/ram/mser");
	benchmark_mser();

	DIRCommand("/ram/pca");
	benchmark_pca();

	uint8_t ucBuffer[ 50 ];

	pxDestinationFile = ff_fopen("/ram/hello.txt", "r");
	int xCount = ff_fread( ucBuffer, 1, 10, pxDestinationFile );
	ff_fclose(pxDestinationFile);

	ucBuffer[11] = '\0';
	ucBuffer[10] = '\n';
	UART_polled_tx_string(gp_my_uart, ucBuffer);

	pxSourceFile = ff_fopen("/ram/abc.txt", "w");
	ff_fwrite("hello fat filesystem\n", 20, 1, pxSourceFile);
	ff_fclose(pxSourceFile);
	
	pxDestinationFile = ff_fopen("/ram/hello.txt", "r");
	xCount = ff_fread( ucBuffer, 1, 10, pxDestinationFile );
	ff_fclose(pxDestinationFile);

	ucBuffer[11] = '\0';
	ucBuffer[10] = '\n';
	UART_polled_tx_string(gp_my_uart, ucBuffer);

	DIRCommand("/ram/");

	while(1) {}

	BaseType_t xResult;

	xResult = xTaskCreate( printTask,
						  "Check",
						  configMINIMAL_STACK_SIZE,
						  NULL,
						  2,
						  NULL );
	vTaskStartScheduler();
    return 0;
}
/*-----------------------------------------------------------*/

static void printTask( void *pvParameters )
{
	TickType_t xNextWakeTime;
	
	for (;;){
		UART_polled_tx_string(gp_my_uart, s_uart);
		vTaskDelay(1000);
	}

}
void vApplicationTickHook( void )
{}



void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	__asm volatile( "ebreak" );
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;
	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	__asm volatile( "ebreak" );
	for( ;; );
}
/*-----------------------------------------------------------*/


void vAssertCalled( void )
{
    uint8_t s_debug[16] = "vassert called\r\n";
    portENTER_CRITICAL();
    UART_polled_tx_string(gp_my_uart, s_debug);
    portEXIT_CRITICAL();
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
	while( ulSetTo1ToExitFunction != 1 )
	{
		__asm volatile( "NOP" );
	}
}
