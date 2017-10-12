% Produce a C model from a Matlab model
function make_C_model(matlab_model_file, output_file)
	if nargin < 2
		disp('Usage:  make_C_model(''matlab_model_file'', ''output_file''');
		disp('Example:  make_C_model(''PARSE_model'', ''cmodel.txt''');
		return
	end
	% compile mex file
	mex -O -DWITH_MATLAB=1 -DFPTYPE=double -I'../src' 'convert_model.cc' '../src/model.cpp';
	% load Matlab model
	load(matlab_model_file);
	% convert
	convert_model(model, output_file);
