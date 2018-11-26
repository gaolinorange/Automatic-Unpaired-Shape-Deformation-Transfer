%% CombineLFDandICP: combine lfd and icp distance
function [] = CombineLFDandICP(dir_name)
  storedStructure = load([dir_name, '\lightfield.mat'], 'dismat');
  lfd_dist = storedStructure.dismat;
  clear('storedStructure');

  storedStructure = load([dir_name, '\icp_dist.mat'], 'dismat');
  icp_dist = storedStructure.dismat;
  clear('storedStructure');

  dismat = lfd_dist/norm(lfd_dist) + icp_dist/norm(icp_dist);
  save([dir_name, '\', 'lfd_icp_dist.mat'], 'dismat');
end