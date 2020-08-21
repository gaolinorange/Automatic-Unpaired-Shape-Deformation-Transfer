function get_feature17(densemesh,simmesh,matpath)
% get the feature with pooling and the convolution
% FLOGRNEW,FS,neighbour1,,neighbour2,mapping,demapping
if nargin<3
    matpath=densemesh;

end

vdensemeshlist=dir([densemesh,'\*.obj']);
[~,i]=sort_nat({vdensemeshlist.name});
vdensemeshlist=vdensemeshlist(i);
fv=load([densemesh,'\simp\FeatureMat1.mat']);

[vdensemesh,~,~,~,~,VVdense,cotweight_dense]=cotlp([densemesh,'\',vdensemeshlist(1).name]);
    if nargin<2
        simmesh=[densemesh,'\',vdensemeshlist(1).name];
    end
[vsimpmesh,~,~,~,~,VVsimp,cotweight_simp]=cotlp(simmesh);

neighbour1=zeros(size(vdensemesh,1),100);
maxnum=0;
for i=1:size(VVdense,1)
    neighbour1(i,1:size(VVdense{i,:},2))=VVdense{i,:};
    if size(VVdense{i,:},2)>maxnum
        maxnum=size(VVdense{i,:},2);
    end
end
neighbour1(:,maxnum+1:end)=[];

idx=knnsearch(vdensemesh,vsimpmesh);


    [ fmlogdr, fms ] = FeatureMap( fv.LOGRNEW, fv.S );
    feature = cat(2, fms, fmlogdr);
    fmlogdr=permute(reshape(fmlogdr,size(fmlogdr,1),3,size(vdensemesh,1)),[1,3,2]);
    fms=permute(reshape(fms,size(fms,1),6,size(vdensemesh,1)),[1,3,2]);
cotweight1=zeros(size(neighbour1));
    for i=1:size(neighbour1,1)
        for j=1:size(neighbour1,2)
            if neighbour1(i,j)>0
%                 cotweight1(i,j)=cotweight_dense(i,neighbour1(i,j));
                cotweight1(i,j)=1/length(nonzeros(neighbour1(i,:)));
            end
        end
    end
%     iii=[1,11:211,264:305];
%     iii=[1:192,238:261];
iii=1:size(fmlogdr,1);
    m=matfile([matpath,'\togFeaturepooling.mat'],'writable',true);
    m.FLOGRNEW=fmlogdr(iii,:,:);
    m.FS=fms(iii,:,:);
    m.neighbour1=neighbour1;
    m.feature=feature(iii,:,:);
    m.cotweight1=cotweight1;
    FLOGRNEW=fmlogdr(iii,:,:);
    FS=fms(iii,:,:);
    neighbour1=neighbour1;
    feature=feature(iii,:,:);
    cotweight1=cotweight1;

save([matpath,'\togFeaturepoolingc.mat'],'FLOGRNEW','FS','neighbour1','feature','cotweight1','-v7.3')

% % load([matpath,'\togFeaturepooling.mat']);
% save([matpath,'\togFeaturepooling.mat'],'-v7.3');



end




