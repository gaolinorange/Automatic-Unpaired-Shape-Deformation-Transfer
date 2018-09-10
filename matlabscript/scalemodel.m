function [] = scalemodel( foldername, uniform,savefolder)
%UNTITLED 此处显示有关此函数的摘要
%   此处显示详细说明
%load obj

if nargin<3
    savefolder=[foldername,'\scale'];
end
if ~exist(savefolder,'dir')
    mkdir(savefolder);
end
meshlist=dir([foldername,'\*.obj']);
[~,i]=sort_nat({meshlist.name});
meshlist=meshlist(i);

center = zeros(3,1);
radius = 0;

for i = 1 : size(meshlist,1)
    objname= [foldername, '\',meshlist(i).name];
    [ v, f] = cotlp( objname );
    v = v';
    if i==1
        if uniform        
        [center,radius] = vermean(v);
        end
    end
    if uniform        
        verts = scalevwithmean( v,center, radius);
    else
        verts = v;
    end
    savename=[savefolder,'\',meshlist(i).name];
    SaveObjT(savename,verts,f)
end

end