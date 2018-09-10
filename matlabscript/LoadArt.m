function [ feavec ] = LoadArt( filename )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
ANGLE = 10;
CAMNUM = 10;
ART_COEF = 35;
fid = fopen(filename,'r');
feavec = fread(fid,ANGLE*CAMNUM*ART_COEF,'unsigned char=>int32');
fclose(fid);
end

