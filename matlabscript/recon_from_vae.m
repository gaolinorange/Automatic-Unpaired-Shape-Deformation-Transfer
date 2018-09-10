function recon_from_vae(obja,objb,folder,step,ciacib)
% recon vae and random vaefrom h5
randomah5=[folder,'\random_gen_a',num2str(step),'.h5'];
randombh5=[folder,'\random_gen_b',num2str(step),'.h5'];
recon_ah5=[folder,'\recon_a',num2str(step),'.h5'];
recon_bh5=[folder,'\recon_b',num2str(step),'.h5'];
[~,random_aname]=fileparts(randomah5);
[~,random_bname]=fileparts(randombh5);
[~,recon_aname]=fileparts(recon_ah5);
[~,recon_bname]=fileparts(recon_bh5);
random_afolder=[folder,'\',random_aname];
random_bfolder=[folder,'\',random_bname];
recon_afolder=[folder,'\',recon_aname];
recon_bfolder=[folder,'\',recon_bname];

if ~exist(random_afolder,'dir')
    mkdir(random_afolder)
end
if ~exist(random_bfolder,'dir')
    mkdir(random_bfolder)
end
if ~exist(recon_afolder,'dir')
    mkdir(recon_afolder)
end
if ~exist(recon_bfolder,'dir')
    mkdir(recon_bfolder)
end

% copyfile(randomah5,[random_afolder,'\random_gen_a',num2str(step),'.h5'])
% copyfile(randombh5,[random_bfolder,'\random_gen_b',num2str(step),'.h5'])
copyfile(recon_ah5,[recon_afolder,'\recon_a',num2str(step),'.h5'])
copyfile(recon_bh5,[recon_bfolder,'\recon_b',num2str(step),'.h5'])

if nargin<5
    % [latent_z]=recon_from_random(obja, random_afolder,random_afolder);
    [latent_z]=recon_from_random(obja, recon_afolder,recon_afolder);
    
    % [latent_z]=recon_from_random(objb, random_bfolder,random_bfolder);
    [latent_z]=recon_from_random(objb, recon_bfolder,recon_bfolder);
else
    % [latent_z]=recon_from_random(obja, random_afolder,random_afolder,ciacib);
    [latent_z]=recon_from_random(obja, recon_afolder,recon_afolder,ciacib);
    
    % [latent_z]=recon_from_random(objb, random_bfolder,random_bfolder,ciacib);
    [latent_z]=recon_from_random(objb, recon_bfolder,recon_bfolder,ciacib);
end

closematlab

end