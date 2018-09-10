function [ feavec ] = LoadEcc( filename )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
ANGLE = 10;
CAMNUM = 10;
fid = fopen(filename,'r');
feavec = fread(fid,ANGLE*CAMNUM,'unsigned char=>int32');
fclose(fid);
end


