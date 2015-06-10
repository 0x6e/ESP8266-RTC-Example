#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"

os_timer_t rtc_test_t;
#define RTC_MAGIC 0x55aaaa55

typedef struct {
  uint64 timeAcc;
  uint32 magic;
  uint32 timeBase;
} RTC_TIMER_DEMO;

void rtcCount()
{
  RTC_TIMER_DEMO rtcTime;
  static uint8 count = 0;
  system_rtc_mem_read(64, &rtcTime, sizeof(rtcTime));

  // Initialise the time struct
  if (rtcTime.magic != RTC_MAGIC)
  {
    os_printf( "rtc time init...\r\n");
    
    rtcTime.magic = RTC_MAGIC;
    rtcTime.timeAcc = 0;
    rtcTime.timeBase = system_get_rtc_time();
    
    os_printf( "time base: %d \r\n", rtcTime.timeBase );
  }

  os_printf("===================\r\n");
  os_printf("RTC time test\r\n");

  uint32 rtcT1, rtcT2;
  uint32 st1, st2;
  uint32 cal1, cal2;

  rtcT1 = system_get_rtc_time();
  st1 = system_get_time();

  cal1 = system_rtc_clock_cali_proc();
  os_delay_us(300);

  st2 = system_get_time();
  rtcT2 = system_get_rtc_time();

  cal2 = system_rtc_clock_cali_proc();

  os_printf("  RTC t2 - t1: %d \r\n", rtcT2 - rtcT1);
  os_printf("  SYS t2 - t1: %d \r\n", st2 - st1);
  os_printf("Cal 1: %d.%d \r\n", ((cal1 * 1000) >> 12) / 1000, ((cal1 * 1000) >> 12) % 1000 );
  os_printf("Cal 2: %d.%d \r\n", ((cal2 * 1000) >> 12) / 1000, ((cal2 * 1000) >> 12) % 1000 );

  os_printf("===================\r\n");
  
  rtcTime.timeAcc += ( ((uint64) (rtcT2 - rtcTime.timeBase)) * ((uint64) ((cal2 * 1000) >> 12)) );

  os_printf("RTC time accuracy: %lld \r\n", rtcTime.timeAcc);
  os_printf("Power on time: \r\n");
  os_printf(" - %lld us\r\n", rtcTime.timeAcc / 1000);
  os_printf(" - %lld.%02lld S\r\n", (rtcTime.timeAcc / 10000000) / 100, (rtcTime.timeAcc / 10000000) % 100);

  rtcTime.timeBase = rtcT2;
  system_rtc_mem_write(64, &rtcTime, sizeof(rtcTime));
  
  os_printf("-----------------------------\r\n");

  if ( 5 == (count++) )
  {
    os_printf("System restart\r\n");
    system_restart();
  }
  else
  {
    os_printf("Continue...\r\n\r\n");
  }
}


void user_init(void)
{
  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  rtcCount();

  os_printf("SDK version:%s\n", system_get_sdk_version());
  
  os_timer_disarm(&rtc_test_t);
  os_timer_setfn(&rtc_test_t, rtcCount, NULL);
  os_timer_arm(&rtc_test_t, 10000, 1);
}

