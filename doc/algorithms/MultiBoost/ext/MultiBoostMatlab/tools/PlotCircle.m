% PlotCircle - plots a circle on your figure
% 
% Call the functions as shown below
% 
% PlotCircle(Column,Row,Radius,N,Color);
% 
% PlotCircle is a function that takes 5 inputs
% 
% Inputs Types
%  ------------
%  Column - Integer, Float
%  Row    - Integer, Float
%  Radius - Integer, Float
%  N      - Integer
%  Color  - Character String
% 
% If your figure will be treated as a Matrix (e.g. image)
% -------------------------------------------------------
% 1. Column - is the column(in the matrix) of the center of the circle (Integer)
% 2. Row    - is the row(in the matrix) of the center of the circle (Integer)
% 3. Radius - is the radius of the required circle (Integer)
% 4. N      - is the number of points that will be used to plot the circle (Integer)
% 5. Color  - is the color of the circle (Character String)
% 
% If your figure will be treated as a Normal Graph
% ------------------------------------------------
% 1. Column - is the co-ordinates of the horizintal axis of the center of the circle (Integer, Float)
% 2. Row    - is the co-ordinates of the vertical axis of the center of the circle (Integer, Float)
% 3. Radius - is the radius of the required circle (Integer, Float)
% 4. N      - is the number of points that will be used to plot the circle (Integer)
% 5. Color  - is the color of the circle (Character String)
% 
% Notes on N: The more you increase N, the more you will get an accurate circle
%             The standard value for N is 256
% 
% Notes on Color: Color is a character string, so you must write the charachter between two ''
% 
%  'b'     blue          
%  'g'     green         
%  'r'     red           
%  'c'     cyan            
%  'm'     magenta       
%  'y'     yellow       
%  'k'     black         
%  'w'     white
%  
% Author: Karim Mansour
% E-mail: karim_mansour@msn.com

function PlotCircle(Column,Row,Radius,N,Color,angle)

if(N<=1)
    error('N must be greater than 1');
end

if (Color ~='b') && (Color ~='g') && (Color ~= 'r') && (Color ~='c') && (Color ~='m') && (Color ~='y') && (Color ~='k') && (Color ~='w')
    error('This is not an available color, Please use help PlotCircle to choose an appropriate color');
end

hold on
t=(0:N)*2*pi/N;
x=Radius*cos(t)+Column;
y=Radius*sin(t)+Row;
plot(x,y,Color,'LineWidth',1);
%fill(x,y,'w','LineWidth',1);

arad=pi*angle/180;
x1=Radius*cos(arad)+Column;
y1=Radius*sin(arad)+Row;
%plot(x1,y1,'r','LineWidth',1);

arad2=pi*(angle+180)/180;
x2=0.7*Radius*cos(arad2)+Column;
y2=0.7*Radius*sin(arad2)+Row;

x=[x1;x2];
y=[y1;y2];
plot(x,y,Color,'LineWidth',2);

arad3=pi*(angle-15)/180;
x3=0.5*Radius*cos(arad3)+Column;
y3=0.5*Radius*sin(arad3)+Row;

x=[x1;x3];
y=[y1;y3];
plot(x,y,Color,'LineWidth',2);

arad4=pi*(angle+15)/180;
x4=0.5*Radius*cos(arad4)+Column;
y4=0.5*Radius*sin(arad4)+Row;

x=[x1;x4];
y=[y1;y4];
plot(x,y,Color,'LineWidth',2);

x=[x3;x4];
y=[y3;y4];
plot(x,y,Color,'LineWidth',2);

t=(0:4)*2*pi/4;
x5=0.7*Radius*cos(t)+Column;
y5=0.7*Radius*sin(t)+Row;

t=(0:4)*2*pi/4;
x6=0.9*Radius*cos(t)+Column;
y6=0.9*Radius*sin(t)+Row;

text(x5(1),y5(1),['\fontsize{7}{\color{green}E}']);
text(x5(2),y5(2),['\fontsize{7}{\color{green}S}']);
text(x6(3),y6(3),['\fontsize{7}{\color{green}W}']);
text(x5(4),y5(4),['\fontsize{7}{\color{green}N}']);
%text(15,15,['\fontsize{10}{\color{green}N}']);

 
