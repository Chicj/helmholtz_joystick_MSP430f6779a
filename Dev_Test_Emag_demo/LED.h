#define LED1    (P7OUT|=BIT7)
#define LED2    (P7OUT|=BIT6)
#define LED3    (P7OUT|=BIT5)
#define LED4    (P7OUT|=BIT4)
#define LED5    (P7OUT|=BIT3)
#define LED6    (P7OUT|=BIT2)
#define LED7    (P7OUT|=BIT1)
#define LED8    (P7OUT|=BIT0)

//leaves MSP "good" led on
#define LEDOFF  (P7OUT=BIT0)

#define LED1OFF    (P7OUT&=~BIT7)
#define LED2OFF    (P7OUT&=~BIT6)
#define LED3OFF    (P7OUT&=~BIT5)
#define LED4OFF    (P7OUT&=~BIT3)
#define LED6OFF    (P7OUT&=~BIT2)
#define LED7OFF    (P7OUT&=~BIT1)
#define LED8OFF    (P7OUT&=~BIT7)
