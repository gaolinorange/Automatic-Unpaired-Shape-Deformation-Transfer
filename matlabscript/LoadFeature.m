function [FeatureVector] = LoadFeature(modelfolder,ModelNum)
%ModelNum = 2201;
ANGLE = 10;
CAMNUM = 10;
CoeffArt = 35;
CoeffFD = 10;
modellist=dir([modelfolder,'\*.obj']);
[~,i]=sort_nat({modellist.name});
modellist=modellist(i);
modellist=modellist(1:ModelNum);
FeatureVector = zeros(ModelNum, (CoeffArt+CoeffFD+2)*ANGLE*CAMNUM,'int32');
AC = ANGLE*CAMNUM;
posArt = zeros(CoeffArt*AC,1);
posFD = zeros(CoeffFD*AC,1);
posCir = zeros(AC,1);
posEcc = zeros(AC,1);
for i = 1 : AC    
    for j = 1:CoeffArt
        posArt((i-1)*CoeffArt+j) = (i-1)*47+j;
    end    
    for j = (CoeffArt+1) : (CoeffArt+CoeffFD)
        posFD((i-1)*CoeffFD+j-CoeffArt) = (i-1)*47+j;
    end
    posCir(i) = (i-1)*47+46;
    posEcc(i) = (i-1)*47+47;    
end
%modelfolder = 'E:\project\exploration\final_db\models\';
%modelfolder ='E:\feature\';
CoeffArtj = ANGLE*CAMNUM;
tic;
for i  = 1 : ModelNum
    i
%     iname = num2str(i);
    [~,iname]=fileparts(modellist(i).name);
    artfile = [modelfolder,'\',iname,'_q8_v1.8.art'];    
    fdfile = [modelfolder,'\',iname,'_q8_v1.8.fd'];
    cirfile = [modelfolder,'\',iname,'_q8_v1.8.cir'];
    eccfile = [modelfolder,'\',iname,'_q8_v1.8.ecc'];
    FeatureVector(i,posArt) = LoadArt(artfile);
    FeatureVector(i,posFD) = LoadFD(fdfile)*2;
    FeatureVector(i,posCir) = LoadCir(cirfile)*2;
    FeatureVector(i,posEcc) = LoadEcc(eccfile);
end
toc;

end