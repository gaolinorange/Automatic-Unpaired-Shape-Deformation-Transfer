function []=visual_error(path1,path2,targetName)
[v1,f1,~]=cotlp(path1);
[v2,f2,~]=cotlp(path2);
v_error=v1-v2;
dist=sum(v_error.*v_error,2);
dist_min=min(dist);
dist_max=max(dist);
dist_n=dist./dist_max;
pointCounts = size(v1,1);
meshCounts = size(f1,2);
rgb=imread('plottingscale.png');
[height,width,channels] = size(rgb);
newPointCounts=pointCounts;
newOBJ = zeros(3, newPointCounts);
colorArr = zeros(3,newPointCounts);
index = 1;
for i = 1 : pointCounts
    P = v1(i,:);
    y=uint32(1024-dist_n(i)*1024);
    if (y < 1)
        y=uint32(1);
    end
    if (y > height)
        y=uint32(1024);
    end
    newOBJ(:,index) = P;
    colorArr(:,index) = [rgb(y,1,1),rgb(y,1,2),rgb(y,1,3)]';
    index = index + 1;
end

fid = fopen(targetName, 'w');
fprintf(fid, 'ply\n');
fprintf(fid, 'format ascii 1.0\n');
fprintf(fid, 'element vertex %d\n', newPointCounts);
fprintf(fid, 'property float x\n');
fprintf(fid, 'property float y\n');
fprintf(fid, 'property float z\n');
fprintf(fid, 'property uchar red\n');
fprintf(fid, 'property uchar green\n');
fprintf(fid, 'property uchar blue\n');
fprintf(fid, 'property uchar alpha\n');
fprintf(fid, 'element face %d\n', meshCounts);
fprintf(fid, 'property list uint8 int32 vertex_index\n');
fprintf(fid, 'end_header\n');

for i = 1 : newPointCounts
    P = newOBJ(:,i);
    color = colorArr(:,i);
    fprintf(fid, '%f %f %f %d %d %d 255\n', P(1,1),P(2,1),P(3,1),color(1,1),color(2,1),color(3,1));
    
end
% for i =1 : meshCounts
%     fprintf(fid, '%d %d %d %d %d %d 255\n', f1(1,i),f1(2,i),f1(3,i),colorArr(1,f1(1,i)),colorArr(2,f1(1,i)),colorArr(3,f1(1,i)));
% end
for i =1 : meshCounts
    fprintf(fid, '%d %d %d %d\n', 3,f1(1,i)-1,f1(2,i)-1,f1(3,i)-1);
end

fclose(fid);


end

