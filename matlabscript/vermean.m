function [ meanv, radius ] = vermean( v )
%MEAN 此处显示有关此函数的摘要
%   此处显示详细说明
[dim,vnum] = size(v);
meanv = mean(v,2);
vnew = v-repmat(meanv,1,vnum);
radius = max(sqrt(sum(vnew.*vnew,1)));
end

