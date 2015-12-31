#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ctl.h>
#include <terminal.h>
#include <commandLib.h>
#include <ARCbus.h>
#include <stdlib.h>
#include <errno.h>
#include <UCA1_uart.h>
#include "pins.h"
#include "coils.h"
#include "LED.h"

char *gets_s(char *buf,unsigned short len){
  register int c;
  register char *s;
  for(s = buf,len--;len &&  (c = getchar()) != '\n' && c!='\r';){
    if (c == EOF){
      if (s == buf){
        return (NULL);
      }else{
        break;
      }
    }else{
      *s++ = c;
      len--;
    }
  }
  *s = 0;
  return (buf);
}

int pwm_cmd(char *argv[],unsigned short argc){
  int pwm[2];
  if (argc!=2){
    printf("error %s requiers 2 args\r\n",argv[0]);
    return -1;
  }
  // string to number
  pwm[0]=atoi(argv[1]);
  pwm[1]=atoi(argv[2]);

  coil_PWM(pwm[0],pwm[1]);
  printf("pwm1=%u pwm2=%u \r\n",COIL1_PWM,COIL2_PWM);
  printf("%i %u %i %u\r\n",(P4OUT&COIL1_DIR_MASK)>>COIL1_DIR_SHIFT,COIL1_PWM,(P4OUT&COIL2_DIR_MASK)>>COIL2_DIR_SHIFT,COIL2_PWM);
  return 0;
}

int dir_cmd(char *argv[],unsigned short argc){
  unsigned short d1,d2;
  if (argc!=2){
    printf("error %s requiers 2 args\r\n",argv[0]);
    return -1;
  }
  d1=atoi(argv[1]);
  d2=atoi(argv[2]);

  P4OUT=(P4OUT&~(COIL1_DIR_MASK|COIL2_DIR_MASK))|(d1<<COIL1_DIR_SHIFT)|(d2<<COIL2_DIR_SHIFT);
  printf("P4OUT = 0x%02X\r\n",P4OUT);
}

int matlab_cmd(char *argv[],unsigned short argc){
  char buffer[sizeof("v -xxxx -xxxx")+5];
  int pwm1,pwm2,num,run=1;
  unsigned short mag;
  float sd;
  double yaw;
  while(run){
    if(gets_s(buffer,sizeof(buffer))==NULL){
      break;
    }
    switch(buffer[0]){
      case 'v':
        // some leds
        LED1;
        LED2OFF;
        //grab serial data from matlab script 
        num=sscanf(buffer,"v %i %i",&pwm1,&pwm2);
        if(num==2){
          //turn off spinning
          coil_spin(0,0);
          //set pwm
          coil_PWM(pwm1,pwm2);
        }else{
          printf("Error : could not parse \"%s\" aborting\r\n",buffer);
          run=0;
        }
      break;
      case 's':
        //some leds
        LED2;
        LED1OFF;
        num=sscanf(buffer,"s %f %u",&sd,&mag);
        if(num==2){
          //init. spin mode
          coil_spin(sd,mag);
       
        }else{
          printf("Error : could not parse \"%s\" aborting\r\n",buffer);
          run=0;
        }
      break;
      case 'q':
        run=0;
      break;
      default:
        //unknown command
      break;
    }
  }
  LED1OFF;
  LED2OFF;
  //turn off spinning
  coil_spin(0,0);
  // turn "off" coil
  coil_PWM(0,0);
}


int spin_cmd(char *argv[],unsigned short argc){
  float rate;
  unsigned int d1,d2;
  unsigned short mag;
  extern float spin_t;
  if (argc!=2){
    printf("error %s requiers 2 args\r\n",argv[0]);
    return -1;
  }
  errno=0;
  rate=strtof(argv[1],NULL);
  if(errno){
    printf("Error could not parse \"%s\".\r\n",argv[1]);
    return -1;
  }
  mag=atoi(argv[2]);
  coil_spin(rate,mag);
  printf("Spinning press any key to exit\r\n");
  while(UCA1_CheckKey()==EOF){
    d1=P4OUT;
    d2=(d1&COIL2_DIR_MASK)>>COIL2_DIR_SHIFT;
    d1=(d1&COIL1_DIR_MASK)>>COIL1_DIR_SHIFT;
    printf("%-2i %-4u %-2i %-4u %f\r\n",d1,COIL1_PWM,d2,COIL2_PWM,spin_t);
    ctl_timeout_wait(ctl_get_current_time()+10);
  }
  printf("Spinning complete\r\n");
  coil_spin(0,0);
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"pwm","",pwm_cmd},
                   {"dir","",dir_cmd},
                   {"matlab","",matlab_cmd},
                   {"spin","",spin_cmd},
                   ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS,
                   //end of list
                   {NULL,NULL,NULL}};

