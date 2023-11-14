function [ret] = N5000_RAW2Conc(RawDataP, RawDataF)
    ret = -1;
%     RawDataP=strcat(pwd, '\.\');
%     RawDataF='rawdata.csv';
    
    R=[RawDataP, RawDataF];
    disp('Loading raw data...');
    R_data = csvread(R,18,0); % Load the .csv file
    t = (R_data(:,1)); % t is required in the file (unit: s)
    R_dataVol = (R_data(:,2:end).*2.5)./(32768-1)+2.5; % transfer the raw ADC output to voltage output
        
    %%%%% convert to dod %%%%%
    d=R_dataVol;
    dm = mean(abs(d),1);
    nTpts = size(d,1);
    dod = -log(abs(d)./(ones(nTpts,1)*dm));
    
    %%%%% convert to concentrations %%%%%
    e = GetE([735 850]);
    e = e(:,1:2) / 10; % convert from /cm to /mm
    einv = inv(e'*e)*e';
    
    ppf=[6.0,6.0];
    rho=2.6;
    R_dataConc=dod;
    for idx=1:(length(dod(1,:))/3)
        idx1=2+(idx-1)*3;
        idx2=idx1+1;
        R_dataConc(:,[idx1 idx2])=( einv * (dod(:,[idx1 idx2])./(ones(nTpts,1)*rho*ppf))' )';
    end
    
    %%%%% save to file %%%%%
    inputFileHandle=fopen(R, 'r');
    if inputFileHandle == -1
        error(['fail to open file:' R]);
    end
    
    outputFileName = [RawDataP, strcat(RawDataF(1:end-4), 'Conc.csv')];
    outputFileHandle = fopen(outputFileName, 'w');
    if outputFileHandle == -1
        error(['fail to open file:' outputFileName]);
    end
    
    for i=1:18
        currentLine = fgetl(inputFileHandle);
        fprintf(outputFileHandle, '%s\n', currentLine);
    end
    
    fclose(inputFileHandle);
    
    for i=1:length(t)
        fprintf(outputFileHandle,'%f,',t(i));
        for j=1:length(R_dataConc(i,:))
            fprintf(outputFileHandle, '%.16g,', R_dataConc(i,j));
        end
        fprintf(outputFileHandle, '\n');
    end
    
    fclose(outputFileHandle);

    ret = 0;
end




