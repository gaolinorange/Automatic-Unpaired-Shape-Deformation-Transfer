function genfeature(modelfolder)
if (~exist([modelfolder,'\simp'],'file'))
    mkdir([modelfolder,'\simp']);
end
cmd=['ACAPOpt.exe ',modelfolder];
dos(cmd);
load([modelfolder,'\FeatureMatgao.mat']);

save([modelfolder,'\simp\FeatureMatgao1.mat'],'LOGRNEW','S');
end