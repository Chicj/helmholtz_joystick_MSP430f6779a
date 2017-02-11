function joystick()
%test scrpit for E-mag demo
%change to the correct MSP comm port to use script
s=serial('COM4','BaudRate',9600);
fopen(s);
%create joystick object 
joy=vrjoystick(1);
history=[];
curr_waveform=[];
figure();
set(gcf, 'Position', get(0,'Screensize'));
MAX_PWM=2000; 
global read_joy;

MAX_HISTORY=50;

% talk to MSP
fprintf(s,'matlab');
%create timers
t=timer('ExecutionMode','fixedRate','Period',.1,'TimerFcn',@joystick_update,'UserData',struct('s',s,'joy',joy,'history',history));


set(gcf, 'DeleteFcn',{@figure_exit,t});
%start timer
start(t);

OSR=256;
GAIN=2;
V_SCALE=1.2/(GAIN*2^(3*log2(OSR)));
R_SENS=2;
CURR_SCALE=V_SCALE/R_SENS;

while(strcmp(t.running,'on'))
    if read_joy
        read_joy=false;   

        %reads axis 1=x,2=y, 3=yaw, 4=throtel axis 
        values=axis(joy,[1,2,3,4]);
        % check button vals 1-->9
        press=button(joy,(1:9));
        if(any(press(3:9))) 
            stop(t);
        end
        %xy and yaw controle(use trigger button, allows rotation of mag field 360deg)
        if press(1)==1
            %save yaw data
            yaw=values(3)*pi;
            %calculate pwm values and add slider intensity controle
            pwm1=cos(yaw)*((1-values(4))/2);
            pwm2=sin(yaw)*((1-values(4))/2);
            str=sprintf('v %i %i',round(pwm1*MAX_PWM),round(pwm2*MAX_PWM));
            %send to controler
            fprintf(s,str);
            %spit vals to terminal
            %fprintf('%s\n',str)
        %spin controle (left joystick button,continuos spin in the "yaw" dir(throtel controles mag.))
        elseif(press(2)==1)
            %passed vals
            sd=values(2);
            %sd=sign(sd)*(0.04^(abs(sd))-1);
            sd=sd*3;
            mag=round(MAX_PWM*(1-values(4))/2);

            str=sprintf('s %f %i',sd,mag);
            %send to controler (speed and dir,magnitud)
            fprintf(s,str);
            %spit vals to terminal
            %fprintf('%s\n',str)
        else
            str=sprintf('v %i %i',round(values([1,2])*MAX_PWM));
            %send x-y data for x-y controle
            fprintf(s,str);
            % spit out to terminal pwm
            %fprintf('%s\n',str)
        end
    
            %format plot xy
        history(:,end+1)=values;
        sz=size(history,2);
        plot([1:sz],history);
        if(sz>1)
            axis([1 sz -1.1 1.1]);
            if(sz>MAX_HISTORY)
                history=history(:,MAX_HISTORY);
            end
        end
        legend ('X axis','Y axis','Yaw axis','Throttle')
        drawnow 
    end
    % curr sens 
%     if s.BytesAvailable>4
%         line=fgetl(s);
%         curr=sscanf(line,'I %d %d');
%         if(length(curr)==2)
%             %apply scale factor
%             curr_waveform(:,end+1)=curr*CURR_SCALE*1e3;
%             sz=size(curr_waveform,2);
%             if(mod(sz,50)==0)
%                 plot([1:sz],curr_waveform);
%                 legend ('X axis','Y axis');
%                 drawnow 
%             end
%         else
%             fprintf(2,'Error : Could not parse line "%s"\n',strtrim(line));
%         end
%     end
end


fprintf(s,'q');
close();
delete(t);
close(joy);
fclose(s);
delete(s);
end

function figure_exit(src,evt,tobj)
    stop(tobj);
end

function joystick_update(tobj,evt)   
   global read_joy
   read_joy=true;
end
