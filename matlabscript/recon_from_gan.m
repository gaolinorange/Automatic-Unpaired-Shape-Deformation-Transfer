function recon_from_gan(obja,objb,folder,step)
IAh5=[folder,'\IA',num2str(step),'.h5'];
IBh5=[folder,'\IB',num2str(step),'.h5'];
agenbh5=[folder,'\a_gen_b',num2str(step),'.h5'];
bgenah5=[folder,'\b_gen_a',num2str(step),'.h5'];
[~,ianame]=fileparts(IAh5);
[~,ibname]=fileparts(IBh5);
[~,agbname]=fileparts(agenbh5);
[~,bganame]=fileparts(bgenah5);
iafolder=[folder,'\',ianame];
ibfolder=[folder,'\',ibname];
agbfolder=[folder,'\',agbname];
bgafolder=[folder,'\',bganame];

if ~exist(iafolder,'dir')
    mkdir(iafolder)
end
if ~exist(ibfolder,'dir')
    mkdir(ibfolder)
end
if ~exist(agbfolder,'dir')
    mkdir(agbfolder)
end
if ~exist(bgafolder,'dir')
    mkdir(bgafolder)
end

copyfile(IAh5,[iafolder,'\IA',num2str(step),'.h5'])
copyfile(IBh5,[ibfolder,'\IB',num2str(step),'.h5'])
copyfile(agenbh5,[agbfolder,'\a_gen_b',num2str(step),'.h5'])
copyfile(bgenah5,[bgafolder,'\b_gen_a',num2str(step),'.h5'])

%[latent_z]=recon_from_random(obja, iafolder,iafolder);
%[latent_z]=recon_from_random(objb, ibfolder,ibfolder);
[latent_z]=recon_from_random(objb, agbfolder,agbfolder);
[latent_z]=recon_from_random(obja, bgafolder,bgafolder);

end