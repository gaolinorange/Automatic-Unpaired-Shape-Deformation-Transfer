function [ vout ] = scalevwithmean( vin,meanv, radius )
%SCALEV 此处显示有关此函数的摘要
%   此处显示详细说明
%[ meanv, radius ] = vermean( vin );
if size(meanv,1) == size(vin,1)
[dim,vnum] = size(vin);
vout = vin - repmat(meanv,1,vnum);    
else
[vnum,dim] = size(vin);
vout = vin - repmat(meanv',vnum,1);    
end
vout = vout/(2*radius);
end

