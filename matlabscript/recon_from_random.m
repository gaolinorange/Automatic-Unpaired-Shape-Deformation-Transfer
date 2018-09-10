function [latent_z]=recon_from_random(firstfile, matfolder,workpath,name,ciacib)
% point feature
if nargin<3
    workpath=[matfolder,'\..\mesha'];
    name='test_mesh';
elseif nargin<4&&nargin>=3
     name='test_mesh';   
end
if nargin<5
    ciacib=1;
end
if ~ischar(name)&&nargin==4
    ciacib=name;
    name='test_mesh';   
end
if ~exist(workpath,'file')
    mkdir(workpath);
end
originfile = firstfile;
if ischar(matfolder)
    matlist=dir([matfolder,'\*.h5']);
else
    matlist=1;
end
NLOGDR=[];
NS=[];
latent_z=[];
for i=1:size(matlist,1)
    if ischar(matfolder)
%         m.latent_z = h5read([matfolder,'\',matlist(i).name],'/latent_z');
        m.test_mesh = h5read([matfolder,'\',matlist(i).name],['/',name]);
%         m=load([matfolder,'\',matlist(i).name]);
%         gen=permute(m.test_mesh,[1,3,2]);
%         m.latent_z=m.latent_z';
        gen=permute(m.test_mesh,[3,1,2]);
        FLOGDR=gen(:,1:3,:);
        if size(gen,2)>3
            FS=gen(:,4:9,:);
        else
            lowbarindex=strfind(matlist(i).name,'_');
            if (ismember('A',matlist(i).name)||ismember('a',matlist(i).name))
               [mat_namea,~]=searchmat(matfolder);
%                mat_namea='X:\sig18aisa\test7\ballsub1.mat';
            end
            if (ismember('B',matlist(i).name)||ismember('b',matlist(i).name))
               [~,mat_namea]=searchmat(matfolder);
%                mat_namea='X:\sig18aisa\test7\ballsub2.mat';
            end
            mat=load(mat_namea);
            FS=permute(mat.FS,[1,3,2]);
            if size(FS,1)~=size(FLOGDR,1)||(ismember('a',matlist(i).name)&&ismember('b',matlist(i).name))
            FS=zeros(size(FLOGDR).*[1,2,1]);
            FS(:,[1,4,6],:)=ones(size(FLOGDR));
            end
        end
        
        FLOGDR=reshape(FLOGDR,size(FLOGDR,1),size(FLOGDR,2)*size(FLOGDR,3));
        FS=reshape(FS,size(FS,1),size(FS,2)*size(FS,3));
    else
        gen=matfolder;
        m.latent_z=[];
        FLOGDR=gen(:,:,1:3);
        FS=gen(:,:,4:9);
    end
    
    [ NLOGDR1, NS1 ] = InverseMap(FLOGDR,FS);
    NLOGDR=[NLOGDR;NLOGDR1];
    NS=[NS;NS1];
%     latent_z=[latent_z;m.latent_z];
end
h5ifo=h5info([matfolder,'\',matlist(i).name]);
if ismember('feature',{h5ifo.Datasets.Name})
    feature=h5read([matfolder,'\',matlist(i).name],'/feature');
else
    feature=[];
end
if size(NS,1)==size(feature,3)&&(ismember('cia',{h5ifo.Datasets.Name})||ismember('cib',{h5ifo.Datasets.Name}))&&ciacib
    [~,name1]=fileparts([matfolder,'\',matlist(i).name]);
    if ismember('a',name1)     
        id=h5read([matfolder,'\',matlist(i).name],'/cia')'+1;
    else
        id=h5read([matfolder,'\',matlist(i).name],'/cib')'+1;
    end
    NLOGDR=NLOGDR(id,:);
    NS=NS(id,:);
else
    id=1:size(NS,1);
end
for i=1:length(id)
    if exist([workpath,'\',sprintf('%05d',id(i)),name,'.obj'],'file')
        continue
    end
%   GenModelR(originfile,[workpath,'\',sprintf('%05d',i),'.obj'],NLOGDR(i,:), NS(i,:),i);
    recon(originfile,[workpath,'\',sprintf('%05d',id(i)),name,'.obj'],NLOGDR(id(i),:), NS(id(i),:));
    %      [v,f]=cotlp(meshname);
    %     showmesh(i).vertices=v;
    %     showmesh(i).face_vert=f';
    %     figure(i)
    %     VisMesh(showmesh(i));
    %     view(180,-90);
    %     print(1000, '-dpng',  [workpath,'\',id,'.png']);
    %     close(figure(i));
end
closematlab
% snapshot
%piliangjietu(matfolder);
%change_pic_to_video(matfolder,'video', 30, 'png')


end