function [ yrBBoxes ] = FetchYRdetectionBBox_HE( rootFolder, extraMargin )
% traverses rootFolder to fetch single view detections

if nargin < 2
    extraMargin = 0;
end

% prepare output arg
headers = {'Subject', 'Action', 'View', 'Frame',...
    'Xmin', 'Ymin', 'Xmax', 'Ymax'};
yrBBoxes = cell(1000000, numel(headers));
yrBBoxes(1,:) = headers;
counter = 2;

% fetch subfolders:
entryList = dir(rootFolder);
subFolders = cell(size(entryList,1), 1);
for i=1:size(entryList,1)
    if (~strncmpi(entryList(i).name,'S',1)) % if it does not start with 'S'
        continue;
    end
    subFolders{i} = entryList(i).name;
end

id = cellfun(@(x) isempty(x), subFolders(:,1));
subFolders(id) = [];

% for each subfolder
for i=1:numel(subFolders)
    subStr = subFolders{i}; % S1_Box_1_C1
    
    subDir = [rootFolder subStr]; 
    fprintf('Processing %s\n', subStr);
    
    % fetch info about sequence    
    splits = strsplit(subStr, '_');    
    subject = splits{1,1}(2);
    action = splits{1,2};
    view = splits{1,4};
    
    % fetch frames
    poses = dir([subDir '/*bmp_Pose.txt']);
    poses = {poses.name};
    
    for f=1:numel(poses)
        % fetch frame
        frame = str2double(poses{f}(3:6));  % im0386.bmp_Pose.txt              
        

        % fetch poses from text
        pose_yr = YRposeFromText([subDir '/' poses{f}]);

        if (numel(pose_yr) == 0)
            continue;
        end 
                
        % get Bounding Box for this pose
        minX = min(pose_yr(:,1));
        maxX = max(pose_yr(:,1));
        minY = min(pose_yr(:,2));
        maxY = max(pose_yr(:,2));
                        
        % record        
        yrBBoxes{counter,1} = subject;
        yrBBoxes{counter,2} = action;
        yrBBoxes{counter,3} = view;
        yrBBoxes{counter,4} = frame;
        yrBBoxes{counter,5} = minX - extraMargin;
        yrBBoxes{counter,6} = minY - extraMargin;
        yrBBoxes{counter,7} = maxX + extraMargin;
        yrBBoxes{counter,8} = maxY + extraMargin;
        
        counter = counter+1;                     
    end
    fprintf('\n');
    
end

yrBBoxes(counter:end,:) = [];

