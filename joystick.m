%test scrpit for E-mag demo
%change comm to use script
s=serial('COM4','BaudRate',9600);
fopen(s);
%create joystick object 
joy=vrjoystick(1);
pres=zeros(1,9);
history=[];
figure();
set(gcf, 'Position', get(0,'Screensize'));
MAX_PWM=2000;

fprintf(s,'matlab');

while(~any(pres(3:9)))
    %reads axis 1=x,2=y, 3=yaw, 4=throtel axis 
    values=axis(joy,[1,2,3,4]);
    % check button vals 1-->9
    pres=button(joy,(1:9));
    str=sprintf('v %i %i',round(values([1,2])*MAX_PWM));
    %send x-y data for x-y controle
    fprintf(s,str);
    % spit out to terminal pwm
    fprintf('%s\n',str)
    
        %format plot xy
    history(:,end+1)=values;
    sz=size(history,2);
    plot([1:sz],history);
    if(sz>1)
        axis([1 sz -1.1 1.1]);
    end
    legend ('X axis','Y axis','Yaw axis','Throttle')
    drawnow 
    pause(.1) 
    
    %yaw controle(use trigger button, allows rotation of mag field 360deg)
    while(pres(1)==1)
        %check vals
        pres=button(joy,(1:9));
        values=axis(joy,[1,2,3,4]);
        %save yaw data
        yaw=values(3)*pi;
        %calculate pwm values and add slider intensity controle
        pwm1=cos(yaw)*((1-values(4))/2);
        pwm2=sin(yaw)*((1-values(4))/2);
        str=sprintf('v %i %i',round(pwm1*MAX_PWM),round(pwm2*MAX_PWM));
        %send to controler
        fprintf(s,str);
        %spit vals to terminal
        fprintf('%s\n',str)
        pres=button(joy,(1:9));
        values=axis(joy,[1,2,3,4]);
        pause(.1)
        
         %format plot for yaw mode 
    history(:,end+1)=values;
    sz=size(history,2);
    plot([1:sz],history);
    if(sz>1)
        axis([1 sz -1.1 1.1]);
    end
    drawnow
    end
    
    %spin controle (left joystick button,continuos spin in the "yaw" dir(throtel controles mag.))
    while(pres(2)==1)
        %check vals
        pres=button(joy,(1:9));
        values=axis(joy,[1,2,3,4]);
        %passed vals
        sd=values(2);
        %sd=sign(sd)*(0.04^(abs(sd))-1);
        sd=sd*4;
        mag=round(MAX_PWM*(1-values(4))/2);
        
        str=sprintf('s %f %i',sd,mag);
        %send to controler (speed and dir,magnitud)
        fprintf(s,str);
        %spit vals to terminal
        fprintf('%s\n',str)
        pause(.1)
        
         %format plot for yaw mode
    history(:,end+1)=values;
    sz=size(history,2);
    plot([1:sz],history);
    if(sz>1)
        axis([1 sz -1.1 1.1]);
    end
    drawnow 
    end
    
end
fprintf(s,'q');
close();
close(joy);
fclose(s);
delete(s);