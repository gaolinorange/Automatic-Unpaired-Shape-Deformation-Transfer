function dismat=metric(modelfolder_a,modelfolder_b)

save_mat = 'catlionlfd.mat'

a_list=dir([modelfolder_a,'\*.obj']);
b_list=dir([modelfolder_b,'\*.obj']);
ModelNum1=size(a_list,1);
ModelNum2=size(b_list,1);

[FeatureVector1] = LoadFeature(modelfolder_a,ModelNum1);
[FeatureVector2] = LoadFeature(modelfolder_b,ModelNum2);
dismat=zeros([ModelNum1,ModelNum2]);
for i = 1:ModelNum1
    for j = 1:ModelNum2
%         [i,j]
        [dismat(i,j),b] = lf(FeatureVector1(i,:),FeatureVector2(j,:));
    end
end

save('lightfeild.mat','dismat')