
function PlotCompass(boundingBox,angle)

angle=angle+90;
width=boundingBox(2)-boundingBox(1);
radio=width/3;
col=boundingBox(1)-width/3;
row=boundingBox(3)-width/3;
Color='g';
N=30;
PlotCircle(col,row,radio,N,Color,angle);