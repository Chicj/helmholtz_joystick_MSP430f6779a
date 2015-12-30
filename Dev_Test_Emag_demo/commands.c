#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ctl.h>
#include <terminal.h>
#include <commandLib.h>
#include <ARCbus.h>
#include <stdlib.h>
#include "pins.h"
#include "coils.h"


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
  int pwm1,pwm2;
  int num;
  int run=1;
  while(run){
    if(gets_s(buffer,sizeof(buffer))==NULL){
      break;
    }
    switch(buffer[0]){
      case 'v':
        //do things
        num=sscanf(buffer,"v %i %i",&pwm1,&pwm2);
        if(num==2){
          //set pwm
          coil_PWM(pwm1,pwm2);
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
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"pwm","",pwm_cmd},
                   {"dir","",dir_cmd},
                   {"matlab","",matlab_cmd},
                   ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS,
                   //end of list
                   {NULL,NULL,NULL}};

