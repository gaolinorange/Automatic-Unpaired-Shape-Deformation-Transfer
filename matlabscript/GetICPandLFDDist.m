function GetICPandLFDDist(src_dir, dst_dir)
  ReadObj2PointCloudAndSave2Mat(src_dir);
  ReadObj2PointCloudAndSave2Mat(dst_dir);
  IcpAlignInDirectory(src_dir, dst_dir);
  metric(src_dir, dst_dir);
  CombineLFDandICP(src_dir, dst_dir);
end