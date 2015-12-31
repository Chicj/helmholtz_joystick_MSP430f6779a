#include <msp430.h>
#include "coils.h"
#include <math.h>
#include "LED.h"


int spin=0;
//unsigned short coil_mag=0;
float coil_sd=0,spin_t=0,coil_mag=0;


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

  
}

void coils_timer_start(void){
  //starts the timer in the mode we selected
  TA0CTL|=MC__UP;
  TA2CTL|=MC__CONTINOUS;
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
