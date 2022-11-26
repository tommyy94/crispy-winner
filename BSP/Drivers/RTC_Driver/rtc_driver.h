#ifndef RTC_H_
#define RTC_H_


#define RTC_SEC_PERIOD    (1u << 0)
#define RTC_SET_TIME      (1u << 1)
#define RTC_RETURN_TIME   (1u << 2)


#define TS_QUEUE_SIZE       (8u)
#define TS_QUEUE_TIMEOUT    (100)


typedef struct
{
	struct Date
	{
        uint8_t  weekDay;           /* Range from 1 to 7 (weekdays)         */
		uint8_t  day;	    /* Range from 1 to 28/29/30/31          */
		uint8_t  month;	    /* Range from 1 to 12                   */
		uint16_t year;	    /* Absolute year >= 1970(such as 2000)  */
	} date;

	struct Time
	{
		uint8_t  seconds;   /* Range from 0 to 59 */
		uint8_t  minutes;   /* Range from 0 to 59 */
		uint8_t  hour;	    /* Range from 0 to 23 */
	} time;
} Calendar_t;


void RTC_Task(void *pvArg);
void RTC_Init(void);


#endif /* RTC_H_ */
