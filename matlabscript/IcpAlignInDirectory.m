%% IcpAlignInDirectory: align each model in directory A to each model in directory B
function [] = IcpAlignInDirectory(src_dir, dst_dir)
%   src_dir_list = dir([src_dir,'\*.obj']);
%   [~,id] = sort_nat({src_dir_list.name});
%   src_dir_list = src_dir_list(id);
%   dst_dir_list = dir([dst_dir,'\*.obj']);
%   [~,id] = sort_nat({dst_dir_list.name});
%   dst_dir_list = dst_dir_list(id);
    storedStructure = load([src_dir, '\PCs.mat'], 'PCs');
    src_pcs = storedStructure.PCs;
    clear('storedStructure');
    storedStructure = load([dst_dir, '\PCs.mat'], 'PCs');
    dst_pcs = storedStructure.PCs;
    clear('storedStructure');
  disp(src_dir);
  disp(dst_dir); 
  dismat = zeros(size(src_pcs, 2), size(dst_pcs, 2));
  for i = 1:size(src_pcs, 2)
    for j = 1:size(dst_pcs, 2)
      disp([i, j]);
%       [src_v, src_f] = cotlp([src_dir, '\', src_dir_list(i).name]);
%       [dst_v, dst_f] = cotlp([dst_dir, '\', dst_dir_list(j).name]);
% 
%       src_pc = pointCloud(src_v);
%       dst_pc = pointCloud(dst_v);

      [transform_matrix, transform_v] = pcregrigid(src_pcs{i}, dst_pcs{j});
      dismat(i, j) = sum(sqrt(sum((transform_v.Location - src_pcs{i}.Location).^2, 2)));
    end
  end
  save([src_dir, '\icp_dist.mat'], 'dismat');
end