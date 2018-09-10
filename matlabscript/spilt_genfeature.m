function spilt_genfeature(modelfolder)
% spilt folder 
modellist=dir([modelfolder,'\*.obj']);
[~,i]=sort_nat({modellist.name});
modellist=modellist(i);
modelnum=size(modellist,1);
foldernum=ceil(modelnum/100);
id=linspace(1,modelnum,foldernum+1);
for i=1:foldernum
    mkdir([modelfolder,'\',num2str(i)])
    folder{i}=[modelfolder,'\',num2str(i)];

    copyfile([modelfolder,'\',modellist(1).name],[modelfolder,'\',num2str(i),'\',modellist(1).name],'f');

    for j=id(i)+1:id(i+1)
        copyfile([modelfolder,'\',modellist(j).name],[modelfolder,'\',num2str(i),'\',modellist(j).name],'f')
    end
end

for i=1:foldernum
    genfeature(folder{i})
end
LOGRNEW=[];
S=[];

for i=1:foldernum
    feature=load([folder{i},'\simp\FeatureMatgao.mat']);
    if i~=1
        feature.LOGRNEW(1,:)=[];
        feature.S(1,:)=[];
    end
    LOGRNEW=[LOGRNEW;feature.LOGRNEW];
    S=[S;feature.S];
end
mkdir([modelfolder,'\simp'])
save([modelfolder,'\simp\FeatureMatgao.mat'],'LOGRNEW','S')

end