%% ReadObj2PointCloudAndSave2Mat: read obj file to point cloud type and save as mat file
function [] = ReadObj2PointCloudAndSave2Mat(dir_name)
  file_list = dir([dir_name,'\*.obj']);
  [~,id] = sort_nat({file_list.name});
  file_list = file_list(id);
  disp(size(file_list, 1));
  PCs = {}
  for i = 1:size(file_list, 1)
    [v, f] = cotlp([dir_name, '\', file_list(i).name]);
    disp([dir_name, '\', file_list(i).name]);
    v_pc = pointCloud(v);
    disp(i);
    PCs{i} = v_pc;
  end
  save([dir_name, '\PCs.mat'], 'PCs');
end