function listname(srcfolder)
% clear
srcfolder=[srcfolder,'\'];
filelist=[srcfolder,'templist.txt'];
filelistnonex=[srcfolder,'list.txt'];

modellist=dir([srcfolder,'*.obj']);
[~,i]=sort_nat({modellist.name});
modellist=modellist(i,:);
modelnum = size(modellist,1);

fid=fopen(filelist,'w');
fidnonex=fopen(filelistnonex,'w');
for i=1:modelnum
    fwrite(fid,[modellist(i).folder,'\',modellist(i).name]);
    fprintf(fid,'\n');
    [~,name]=fileparts([modellist(i).folder,'\',modellist(i).name]);
    fwrite(fidnonex,[modellist(i).folder,'\',name]);
    fprintf(fidnonex,'\n');
end
fclose(fidnonex);
fclose(fid);
copyfile(filelistnonex,['..\lightfeild\list.txt'],'f');
end