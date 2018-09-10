function [ feavec ] = LoadFD( filename )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
ANGLE = 10;
CAMNUM = 10;
FD_COEF = 10;
fid = fopen(filename,'r');
feavec = fread(fid,ANGLE*CAMNUM*FD_COEF,'unsigned char=>int32');
fclose(fid);
end
