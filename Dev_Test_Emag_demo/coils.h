#ifndef __COIL_H
#define __COIL_H

#define COIL_PWM_PER (2000)

#define COIL1_PWM   TA0CCR1
#define COIL2_PWM   TA0CCR2

#define COIL1_DIR_MASK    (BIT3|BIT4)
#define COIL1_DIR_FWD     (BIT4)
#define COIL1_DIR_REV     (BIT3)
#define COIL1_DIR_SHIFT   (3)

#define COIL2_DIR_MASK    (BIT1|BIT2)
#define COIL2_DIR_FWD     (BIT2)
#define COIL2_DIR_REV     (BIT1)
#define COIL2_DIR_SHIFT   (1)
#define COIL_SPIN_PER     (33)


#define set_dir(c,d)      do{P4OUT|=COIL##c##_DIR_##d;P4OUT&=~(COIL##c##_DIR_MASK^COIL##c##_DIR_##d);}while(0)

#define PI                (3.14159265359)

extern int spin;

//function prototype,lets header files know about functions
void coils_init(void);
void coils_timer_start(void);
void coil_PWM(short pwm1,short pwm2);
void coil_spin(float sd,unsigned short mag);

#endif 
