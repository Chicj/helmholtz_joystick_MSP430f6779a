#include <msp430.h>
#include "coils.h"
#include <math.h>
#include "LED.h"
#include <ctl.h>
#include <ARCbus.h>
#include <stdio.h>

long curr[2];

// makes task structer
CTL_TASK_t ADC_task;
//stack for ADC
unsigned curr_stack[600];
CTL_EVENT_SET_t SD24_events;

int spin=0;
//unsigned short coil_mag=0;
float coil_sd=0,spin_t=0,coil_mag=0;
//events for SD24
enum {SD24_OVERFLOW_EVT=SD24OVIFG0};

// init. timers using 4.0,4.1 | IO pins enagble and dir. | 
void coils_init(void){
  // TA0_CRRx
  TA0CTL=TASSEL__SMCLK|ID_0|TACLR;
  //CCR0
  TA0CCR0=COIL_PWM_PER;
  //CRR1
  TA0CCR1=0;
  TA0CCTL1=OUTMOD_7;
  //CRR2
  TA0CCR2=0;
  TA0CCTL2=OUTMOD_7;
  
  //TA2_CCRx
  TA2CTL=TASSEL__ACLK|ID_0|TACLR;
  //CRR0
  TA2CCR0=COIL_SPIN_PER;
  //set interupt 
  TA2CCTL0=CCIE;

 // port mapping 
  // unlock port mapping functions
  PMAPKEYID=PMAPKEY;
  // allow reconfig. later
  PMAPCTL|=PMAPRECFG;
  // mapps outputs to pins 4.0(CCR1) and 4.1(CCR2)
  P3MAP7=PM_TA0_1;
  P4MAP0=PM_TA0_2;
  //re lock portmapping
  PMAPKEYID=0;
 //configure pins 
  P3SEL0|=BIT7;
  P3DIR|=BIT7;
  P4SEL0|=BIT0;
  P4DIR|=BIT0;
// p4.2-->5 gpio for direction control 
  P4OUT |=  BIT1|BIT3;
  P4OUT &=~(BIT2|BIT4);
  P4SEL0&=~(BIT1|BIT2|BIT3|BIT4);
  P4DIR |= (BIT1|BIT2|BIT3|BIT4);

//ADC inits
  //set ADC settings
  //SD24BCTL0=SD24PDIV_0|SD24SSEL__ACLK|SD24REFS|SD24OV32;
  SD24BCTL0=SD24PDIV_4|SD24DIV1|SD24DIV2|SD24SSEL__SMCLK|SD24REFS|SD24OV32;
  SD24BCTL1=0;
  //Gain settings
  SD24BINCTL3=SD24GAIN_2;
  SD24BINCTL4=SD24GAIN_2;
  //setup ADCs to test
  SD24BCCTL3=SD24DF_1|SD24SCS__GROUP0;  
  SD24BOSR3=256;
  SD24BPRE3=0;  
  SD24BCCTL4=SD24DF_1|SD24SCS__GROUP0;  
  SD24BOSR4=256;
  SD24BPRE4=0;  
  //clear interrupt flags
  SD24BIFG=0;
  //enable interrupts 
  SD24BIE=(SD24OVIE3|SD24IE3)|(SD24OVIE4|SD24IE4);  
}

// sample ADC for curr (toplevel never returns)
void curr_sens(void *p) __toplevel{
  unsigned int e;
  for(;;){
  // sample ADC
    e=ctl_events_wait(CTL_EVENT_WAIT_ALL_EVENTS_WITH_AUTO_CLEAR,&SD24_events,(SD24IFG3|SD24IFG4),CTL_TIMEOUT_NONE,512);
    if (e==(SD24IFG3|SD24IFG4)){
      printf("I %li %li\r\n",curr[0],curr[1]);
    }
  }
}

void coils_start(void){
  //starts the timer in the mode we selected
  TA0CTL|=MC__UP;
  TA2CTL|=MC__CONTINOUS;
  //start ADC task
  //(address of task struc, priority, function to call, pass argument to function,name in the debugger,how many bytes are in the stack,pointer to the stack,size of IRQ (N/A))
  ctl_task_run(&ADC_task,BUS_PRI_LOW+2,curr_sens,NULL,"curr_sens",sizeof(curr_stack)/sizeof(curr_stack[0]),curr_stack,0);

}

void coil_PWM(short pwm1,short pwm2){
  // check driver dir.
  if(pwm1<0){
    set_dir(1,REV);
    pwm1*=-1;
  }else{ 
    set_dir(1,FWD);
  }
  if(pwm2<0){
    set_dir(2,REV);
    pwm2*=-1;
  }else{ 
    set_dir(2,FWD);
  }
  // sets PWM to user def.
  COIL1_PWM=pwm1;
  COIL2_PWM=pwm2;
 }


  // inturupt for ADC 
void SD24_ISR(void) __ctl_interrupt[SD24B_VECTOR]{
  long val;
  switch(SD24BIV){
    case SD24BIV_SD24OVIFG:
      ctl_events_set_clear(&SD24_events,SD24_OVERFLOW_EVT,0);
      //clear interrupts
      //SD24BIFG&=~(SD24OVIFG3|SD24OVIFG4);
      SD24BIV=0;
    return;
    case SD24BIV_SD24TRGIFG:
    break;
    case SD24BIV_SD24IFG3:
      ctl_events_set_clear(&SD24_events,SD24IFG3,0);
      //read and store value
      val =(unsigned long)SD24BMEML3;
      val|=((unsigned long)SD24BMEMH3)<<16;
      curr[0]=val;

    return;
    case SD24BIV_SD24IFG4:
      ctl_events_set_clear(&SD24_events,SD24IFG4,0);
      //read and store value
      val =(unsigned long)SD24BMEML4;
      val|=((unsigned long)SD24BMEMH4)<<16;
      curr[1]=val;
    return;
    default:
      SD24BIV=0;
    break;
  }
}
  

void spin_interupt(void) __ctl_interrupt[TIMER2_A0_VECTOR]{
  //led for time
  LED3;
  // resets the intrrupt 
  TA2CCR0+=COIL_SPIN_PER;
  // check for spin mode
  if (spin){
    // increments mag field dir. (fmod makes sure nothing is overrun)
    spin_t=fmod(coil_sd+spin_t,PI*2); 
    // drive coils
    coil_PWM(coil_mag*sin(spin_t),coil_mag*cos(spin_t));
  }
  LED3OFF;
}

void coil_spin(float sd,unsigned short mag){
  // set sd and mag globally
  coil_mag=mag;
  coil_sd=sd*COIL_SPIN_PER/32.768e3;
  if(sd!=0){
    //set spin mode
    spin=1;
  }else{
    //set spin to zero
    spin=0;
    coil_PWM(0,0);
  }
}

void coil_ADC_start(void){
  SD24BCTL1|=SD24GRP0SC;
}

void coil_ADC_stop(void){
  SD24BCTL1&=~SD24GRP0SC;
  ctl_events_wait(CTL_EVENT_WAIT_ALL_EVENTS_WITH_AUTO_CLEAR,&SD24_events,SD24IFG3|SD24IFG4,CTL_TIMEOUT_DELAY,3);
}
