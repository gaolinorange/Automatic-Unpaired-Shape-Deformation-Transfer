function genfeature(modelfolder)
if (~exist([modelfolder,'\simp'],'file'))
    mkdir([modelfolder,'\simp']);
end
cmd=['ACAPOpt.exe ',modelfolder];
dos(cmd);
%load([modelfolder,'\FeatureMat1.mat']);

%save([modelfolder,'\simp\FeatureMat1.mat'],'LOGRNEW','S');
end