function [ yrBBoxes ] = FetchYRdetectionBBox_UMPM( rootFolder, extraMargin )
% traverses rootFolder to fetch single view detections

if nargin < 2
    extraMargin = 0;
end

% prepare output arg
headers = {'Action', 'View', 'Frame',...
    'Xmin', 'Ymin', 'Xmax', 'Ymax'};
yrBBoxes = cell(1000000, numel(headers));
yrBBoxes(1,:) = headers;
counter = 2;


% fetch subfolders:
entryList = dir(rootFolder);
subFolders = cell(size(entryList,1), 1);
for i=1:size(entryList,1)
    if (~strncmpi(entryList(i).name,'p',1)) % if it does not start with a p
        continue;
    end
    subFolders{i} = entryList(i).name;
end

id = cellfun(@(x) isempty(x), subFolders(:,1));
subFolders(id) = [];

% for each subfolder
for i=1:numel(subFolders)
    subStr = subFolders{i};
    subDir = [rootFolder subStr]; % p1_chair_2_f
    fprintf('Processing %s\n', subStr);
    
    % fetch info about sequence    
    view = subStr(end);       
    
    action = subStr(1:end-2);    
    
    % fetch frames
    poses = dir([subDir '/*png_Pose.txt']);
    poses = {poses.name};
    
    for f=1:numel(poses)
        % fetch frame
        frame = str2double(poses{f}(3:6));        
                      
        % fetch poses from text
        pose_yr = YRposeFromText([subDir '/' poses{f}]);        
                                                        
        % get Bounding Box for this pose
        minX = min(pose_yr(:,1));
        maxX = max(pose_yr(:,1));
        minY = min(pose_yr(:,2));
        maxY = max(pose_yr(:,2));
              
        % record                
        yrBBoxes{counter,1} = action;
        yrBBoxes{counter,2} = view;
        yrBBoxes{counter,3} = frame;
        yrBBoxes{counter,4} = minX - extraMargin;
        yrBBoxes{counter,5} = minY - extraMargin;
        yrBBoxes{counter,6} = maxX + extraMargin;
        yrBBoxes{counter,7} = maxY + extraMargin;
        
        counter = counter+1;                  
    end
    fprintf('\n');
end

% remove empty cells
fprintf('Clean up...\n');
yrBBoxes(counter:end,:) = [];


end

