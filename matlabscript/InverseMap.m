function [ NLOGDR, NS ] = InverseMap( fmlogdr, fms )
%UNTITLED2 此处显示有关此函数的摘要
%   此处显示详细说明
[mfmlogdr,nfmlogdr] = size(fmlogdr);
[mfms, nfms] = size(fms);
edgenum = nfmlogdr/3;
snum = nfms/6;
NLOGDR = zeros(mfmlogdr, edgenum*9);
NS = zeros(mfms, snum*9);

for i = 1 : mfmlogdr
    for j = 0 : edgenum-1
        NLOGDR(i,j*9+1) = 0;        
        NLOGDR(i,j*9+2) = fmlogdr(i,j*3+1);
        NLOGDR(i,j*9+3) = fmlogdr(i,j*3+2);
        NLOGDR(i,j*9+4) = -fmlogdr(i,j*3+1);
        NLOGDR(i,j*9+5) = 0;
        NLOGDR(i,j*9+6) = fmlogdr(i,j*3+3);
        NLOGDR(i,j*9+7) = -fmlogdr(i,j*3+2);
        NLOGDR(i,j*9+8) = -fmlogdr(i,j*3+3);
        NLOGDR(i,j*9+9) = 0;
    end
end

for i = 1 : mfms
    for j = 0 : snum-1
        NS(i,j*9+1) = fms(i,j*6+1);        
        NS(i,j*9+2) = fms(i,j*6+2);
        NS(i,j*9+3) = fms(i,j*6+3);
        NS(i,j*9+4) = fms(i,j*6+2);
        NS(i,j*9+5) = fms(i,j*6+4);
        NS(i,j*9+6) = fms(i,j*6+5);
        NS(i,j*9+7) = fms(i,j*6+3);
        NS(i,j*9+8) = fms(i,j*6+5);
        NS(i,j*9+9) = fms(i,j*6+6);        
    end
end

end

