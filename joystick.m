%test scrpit for E-mag demo
%change comm to use script
s=serial('COM4','BaudRate',9600);
fopen(s);
%create joystick object 
joy=vrjoystick(1);
pres=0;
history=[];
figure();
set(gcf, 'Position', get(0,'Screensize'));
MAX_PWM=2000;

fprintf(s,'matlab');

while(~any(pres))
    %reads axis 1=x,2=y, 3=yaw, 4=throtel axis 
    values=axis(joy,[1,2,3,4]);
    % check button vals 1-->9
    pres=button(joy,(1:9));
    
    fprintf(s,sprintf('v %i %i',round(values([1,2])*MAX_PWM)));
    %format plot
    history(:,end+1)=values;
    sz=size(history,2);
    plot([1:sz],history);
    if(sz>1)
        axis([1 sz -1.1 1.1]);
    end
    legend ('X axis','Y axis','Yaw axis','Throttle')
    drawnow 
    pause(.1) 
end
fprintf(s,'q');
close();
close(joy);
fclose(s);
delete(s);