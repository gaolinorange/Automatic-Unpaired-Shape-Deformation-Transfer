function [FeatureVector] = LoadFeature(modelfolder,ModelNum)
%ModelNum = 2201;
ANGLE = 10;
CAMNUM = 10;
CoeffArt = 35;
CoeffFD = 10;
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
CoeffArtj = ANGLE*CAMNUM;
modelNames = getFileNamesFromDirectory(modelfolder,'types',{''});
modelNames = modelNames(3:end);
tic
for i  = 1 : ModelNum
    i
    %iname = num2str(i);
    artfile = [modelfolder,modelNames{i},'\model_q8_v1.8.art'];    
    fdfile = [modelfolder,modelNames{i},'\model_q8_v1.8.fd'];
    cirfile = [modelfolder,modelNames{i},'\model_q8_v1.8.cir'];
    eccfile = [modelfolder,modelNames{i},'\model_q8_v1.8.ecc'];
    FeatureVector(i,posArt) = LoadArt(artfile);
    FeatureVector(i,posFD) = LoadFD(fdfile)*2;
    FeatureVector(i,posCir) = LoadCir(cirfile)*2;
    FeatureVector(i,posEcc) = LoadEcc(eccfile);
end
toc;

end