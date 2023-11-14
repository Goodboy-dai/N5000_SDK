disp('Open the raw data file (.csv)...');
[RawDataF,RawDataP]=uigetfile('*.csv','Pick the raw data','MultiSelect', 'on','C:\Users');

Mul_F=cell(1);
if isequal(RawDataF,0)
    error('Empty raw data file!');
else
    if ~iscell(RawDataF)
        Sin_F=[RawDataP RawDataF];
    else
        disp(1);
        for i=1:1:size(RawDataF,2)
           Mul_F{i}=[RawDataP RawDataF{i}]; 
        end
    end
    if ~iscell(RawDataF)
        R=Sin_F;
    else
        R=Mul_F;
    end
    disp('Your raw data file is:')
    disp(R);
end

disp('Start converting...');
if iscell(R)    %多个文件
    for i=1:1:size(R,2)
        N5000_RAW2Conc(RawDataP, RawDataF{i});
    end
else %单个文件
    N5000_RAW2Conc(RawDataP, RawDataF);
end
disp('Done!');



