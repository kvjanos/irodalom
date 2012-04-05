function[trainnetout,stopnetout,testnetout,trainclasses,stopclasses,testclasses,trainidx,stopidx,testidx,imin,net]=mb_mlptrainstoptest(features,outputs,classes,hidden,trainsize,stopsize,epochs,permutations)
%  MB_MLPTRAINSTOPTEST - Train/stop/test a multilayer perceptron from NETLAB
%
%  [TRAINNETOUT,STOPNETOUT,TESTNETOUT,TRAINCLASSES,STOPCLASSES,...
%   TESTCLASSES,TRAINIDX,STOPIDX,TESTIDX,IMIN,NET] = ... 
%  MB_MLPTRAINSTOPTEST(FEATURES,OUTPUTS,CLASSES,HIDDEN,TRAINSIZE,
%   STOPSIZE,EPOCHS,PERMUTATIONS)
%
%    Outputs:
%     TRAINNETOUT - 1xN cell array of output from the training samples 
%                    applied to each trained network, where N=PERMUTATIONS
%     STOPNETOUT - 1xN cell array of output from stop samples
%     TESTNETOUT - 1xN cell array of output from test samples
%     TRAINCLASSES - 1xN cell array of classes for training data used with 
%                     each of the networks
%     STOPCLASSES - 1xN cell array of classes for stop data
%     TESTCLASSES - 1xN cell array of classes for test data
%     TRAINIDX - {N}x{N} cell array of random indices used to indicate
%                 training samples
%     STOPIDX - {C}x{N} cell array of random indices for stop samples
%     TESTIDX - {C}x{N} cell array of random indices for test samples
%     IMIN - 1xN cell array of indices at which sum(error^2) was minimized
%     NET - 1xN cell array of trained networks
%
%    Inputs:
%     FEATURES - 1xC cell array of features (C=number of classes)
%     OUTPUTS - 1xC cell array of desired network outputs
%     CLASSES - 1xC cell array of classes (one-of-N format)
%     HIDDEN - number of hidden nodes to use
%     TRAINSIZE - number of training samples per permutation
%     STOPSIZE - number of stop samples per permutation
%     EPOCHS - number of epochs to train the network before 
%               looking for a minimum on the stop data
%     PERMUTATIONS - number of networks to train
%
%
% M. Boland - 13 Apr 1999


% $Id: mb_mlptrainstoptest.m,v 1.7 1999/05/25 21:34:02 boland Exp $

if (~iscell(features) | ~iscell(outputs) | ~iscell(classes)),
  error('FEATURES, OUTPUTS, and CLASSES must be cell arrays.') ;
end

if ((length(features)~=length(outputs)) | ...
    (length(outputs)~=length(classes))),
  error('FEATURES, OUTPUTS, and CLASSES must have the same number of elements') ;
end

if (~isnumeric(permutations)),
  error('PERMUTATIONS must be a single number.') ;
end

%
% Reset the random number generator
rand('state',0) ;

%
% Number of classes
numc = length(features) ;

for i = 1:permutations,
  clear idx ;
  
  trainin  = [] ;
  stopin   = [] ;
  testin   = [] ;
  trainout = [] ;
  stopout  = [] ;
  testout  = [] ;
  trainclass = [] ;
  stopclass  = [] ;
  testclass  = [] ;

  %
  % Generate train/stop/test data for each class (j) for each network (i)
  for j = 1:numc,
    %
    % new random indices for each class
    idx{j}   = randperm(size(features{j},1)) ;

    %
    % Save the indices so that we know which samples were placed in which 
    %  category (train/stop/test)
    trainidx{j}{i} = idx{j}(1:trainsize) ;
    stopidx{j}{i} = idx{j}(trainsize+1:trainsize+stopsize) ;
    testidx{j}{i} = idx{j}(trainsize+stopsize+1:end) ;

    trainin  = [trainin ; features{j}(idx{j}(1:trainsize),:)] ;
    stopin  = [stopin ; features{j}(idx{j}(trainsize+1:trainsize+stopsize),:)];
    testin   = [testin ; features{j}(idx{j}(trainsize+stopsize+1:end),:)] ;

    trainout = [trainout ; outputs{j}(idx{j}(1:trainsize),:)] ;
    stopout = [stopout ; outputs{j}(idx{j}(trainsize+1:trainsize+stopsize),:)];
    testout  = [testout ; outputs{j}(idx{j}(trainsize+stopsize+1:end),:)] ;

    trainclass = [trainclass ; classes{j}(idx{j}(1:trainsize),:)] ;
    stopclass = [stopclass ; classes{j}(idx{j}(trainsize+1:trainsize+stopsize),:)];
    testclass = [testclass ; classes{j}(idx{j}(trainsize+stopsize+1:end),:)] ;
  end

  %
  % Normalize the input features
  [trainnorm stopnorm] = mb_featurenorm(trainin,stopin) ;
  [trainnorm testnorm] = mb_featurenorm(trainin,testin) ;

  %
  % Use mb_mlptraintest for each train/stop pair
  [trainnetout{i},stopnetout{i},imin{i},net{i}] = ...
    mb_mlptraintest(trainnorm, trainout, stopnorm, stopout, hidden, epochs) ;

  %
  % Generate the test outputs
  testnetout{i}=mlpfwd(net{i},testnorm) ;

  trainclasses{i}=trainclass ;
  stopclasses{i}=stopclass ;
  testclasses{i}=testclass ;
end

